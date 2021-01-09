#include "voxelio/format/vox.hpp"

#include "voxelio/color.hpp"
#include "voxelio/filetype.hpp"
#include "voxelio/intdiv.hpp"
#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"
#include "voxelio/parse.hpp"
#include "voxelio/stringify.hpp"
#include "voxelio/stringmanip.hpp"

#include <set>
#include <sstream>

#define LOG_CURRENT_VOXEL_CHUNK()                                                                                     \
    VXIO_LOG(SPAM,                                                                                                    \
             "now reading voxel chunk " + stringify(state.modelIndex + 1) + "/" + stringify(voxelChunkInfos.size()) + \
                 " - " + voxelChunkInfos[state.modelIndex].toString() + " @" + stringify(stream.position()));

#define LOG_CURRENT_SHAPE() VXIO_LOG(SPAM, "now reading shape index " + stringify(state.parentIndex));

namespace voxelio::vox {

struct ChunkHeader {
    ChunkType type;
    u32 selfSize;
    u32 childrenSize;

    u32 totalSize() const
    {
        return selfSize + childrenSize;
    }
};

static const std::set<ChunkType> CHUNK_TYPE_VALUES_SET{CHUNK_TYPE_VALUES.begin(), CHUNK_TYPE_VALUES.end()};

static bool isValidChunkType(u32 type)
{
    return CHUNK_TYPE_VALUES_SET.find(static_cast<ChunkType>(type)) != CHUNK_TYPE_VALUES_SET.end();
}

constexpr const char *nameOf(ChunkType type)
{
    switch (type) {
    case ChunkType::MAIN: return "MAIN";
    case ChunkType::SIZE: return "SIZE";
    case ChunkType::XYZI: return "XYZI";
    case ChunkType::RGBA: return "RGBA";
    case ChunkType::MATT: return "MATT";
    case ChunkType::PACK: return "PACK";
    case ChunkType::nGRP: return "nGRP";
    case ChunkType::nSHP: return "nSHP";
    case ChunkType::nTRN: return "nTRN";
    case ChunkType::MATL: return "MATL";
    case ChunkType::LAYR: return "LAYR";
    case ChunkType::IMAP: return "IMAP";
    case ChunkType::rOBJ: return "rOBJ";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
    return nullptr;
}

constexpr const char *prettyNameOf(ChunkType type)
{
    switch (type) {
    case ChunkType::MAIN: return "Main";
    case ChunkType::SIZE: return "Model Size";
    case ChunkType::XYZI: return "Model Voxels";
    case ChunkType::RGBA: return "RGBA-Color";
    case ChunkType::MATT: return "Deprecated Material";
    case ChunkType::PACK: return "Pack";
    case ChunkType::nTRN: return "Transform Node";
    case ChunkType::nGRP: return "Group Node";
    case ChunkType::nSHP: return "Shape Node";
    case ChunkType::LAYR: return "Layer";
    case ChunkType::MATL: return "Material";
    case ChunkType::IMAP: return "IMAP (?)";
    case ChunkType::rOBJ: return "Renderer Settings";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
    return nullptr;
}

// unused for now, but may be used later
#if 0
constexpr static const char *nameOf(NodeType type)
{
    switch (type) {
    case NodeType::GROUP: return "GROUP";
    case NodeType::SHAPE: return "SHAPE";
    case NodeType::TRANSFORM: return "TRANSFORM";
    }
    DEBUG_ASSERT_UNREACHABLE();
    return nullptr;
}

constexpr static bool isDeprecated(ChunkType type)
{
    return type == ChunkType::MATT;
}
#endif

constexpr static const char *voxNameOf(NodeType type)
{
    switch (type) {
    case NodeType::GROUP: return "nGRP";
    case NodeType::SHAPE: return "nSHP";
    case NodeType::TRANSFORM: return "nTRN";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
    return nullptr;
}

constexpr const char *MAGIC = magicOf(FileType::MAGICA_VOX);
constexpr size_t MAGIC_LENGTH = CHUNK_NAME_LENGTH;

constexpr u32 CURRENT_VERSION = 150;

constexpr const char *KEY_ROTATION = "_r";
constexpr const char *KEY_TRANSLATION = "_t";

float Reader::progress() noexcept
{
    if (dataLength == DATA_LENGTH_UNKNOWN || not initialized) {
        return std::numeric_limits<float>::signaling_NaN();
    }
    return static_cast<float>(stream.position() + 1) / dataLength;
}

ReadResult Reader::init() noexcept
{
    if (initialized) {
        return {0, ResultCode::WARNING_DOUBLE_INIT};
    }
    VXIO_FORWARD_ERROR(readMagicAndVersion());
    VXIO_FORWARD_ERROR(readChunk(false));  // main chunk, eof not allowed because it must exist
    while (not stream.eof()) {
        // eof is allowed for all other chunks
        // we are just reading past the main chunk and expecting an eof to occur
        // TODO consider reading only one byte instead of trying to read more chunks
        VXIO_FORWARD_ERROR(readChunk(true));
    }
    VXIO_LOG(DEBUG, "first/init pass of VOX complete, reader initialized");
    VXIO_DEBUG_ASSERT(stream.eof());
    // we can't seek after eof without clearing the flags
    stream.clearErrors();
    stream.seekAbsolute(voxelChunkInfos[0].pos);
    VXIO_DEBUG_ASSERT(stream.good());
    VXIO_DEBUG_ASSERT_EQ(stream.position(), voxelChunkInfos[0].pos);

    VXIO_FORWARD_ERROR(processSceneGraph());

    LOG_CURRENT_VOXEL_CHUNK();
    LOG_CURRENT_SHAPE();
    updateTransformForCurrentShape();

    initialized = true;
    return ReadResult::ok();
}

ReadResult Reader::processSceneGraph() noexcept
{
    for (u32 shapeNodeId : shapeNodeIds) {
        u32 modelId = nodeMap.find(shapeNodeId)->second.contentId;
        auto [parentsBegin, parentsEnd] = nodeParentMap.equal_range(shapeNodeId);

        for (auto &parentIter = parentsBegin; parentIter != parentsEnd; ++parentIter) {
            u32 parentNodeId = parentIter->second;
            if (auto parentType = nodeMap.find(parentNodeId)->second.type; parentType != NodeType::TRANSFORM) {
                return ReadResult::parseError(
                    stream.position(),
                    "Parent of nSHP expected to be nTRN but was " + std::string{voxNameOf(parentType)});
            }
            voxelChunkInfos[modelId].parentIds.push_back(parentNodeId);
        }
    }
    return ReadResult::ok();
}

ReadResult Reader::read(Voxel32 buffer[], size_t bufferLength)
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::vox::Reader::init()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::vox::Reader::init()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

Vec3i8 Transformation::row(size_t index) const
{
    VXIO_DEBUG_ASSERT_LT(index, 3);
    return matrix[index];
}

Vec3i8 Transformation::col(size_t index) const
{
    VXIO_DEBUG_ASSERT_LT(index, 3);
    return {matrix[0][index], matrix[1][index], matrix[2][index]};
}

Transformation Transformation::concat(const Transformation &lhs, const Transformation &rhs)
{
    Vec3i32 resultTranslation = lhs.translation;
    std::array<Vec3i8, 3> resultMatrix;
    for (size_t row = 0; row < 3; ++row) {
        const auto lhsRow = lhs.row(row);
        for (size_t col = 0; col < 3; ++col) {
            resultMatrix[row][col] = dot(lhsRow, rhs.col(col));
        }
        resultTranslation[row] += dot(lhsRow, rhs.translation);
    }

    return {std::move(resultMatrix), resultTranslation};
}

Vec3i32 Transformation::apply(const Vec3u32 &pointInModel, const Vec3u32 &doublePivot) const
{
    Vec3i32 doublePointRelToCenter = (pointInModel * 2).cast<i32>() - doublePivot.cast<i32>();
    Vec3i32 rotated{};
    for (size_t row = 0; row < matrix.size(); ++row) {
        rotated[row] = divFloor(dot(matrix[row], doublePointRelToCenter), 2u);
    }
    return rotated + translation;
}

/*
 * This implementation is based on the statements made by ephtracy.
 * transform * ( v - ( modelSize / 2 ) )
Vec3i32 Transformation::apply(const Vec3u32 &pointInChunk, const Vec3u32 &chunkSize) const
{
    Vec3i32 pRelToCenter = static_vec_cast<i32>(pointInChunk) - static_vec_cast<i32>(chunkSize / 2);
    Vec3i32 rotated{};
    for (size_t row = 0; row < matrix.size(); ++row) {
        rotated[row] = matrix[row] * pRelToCenter;
    }
    return rotated + translation;
}
*/

std::string Transformation::toString() const
{
    std::stringstream stream;
    stream << "Transformation{r={";
    for (size_t i = 0; i < 3; ++i) {
        const auto &row = matrix[i];
        stream << stringify(row[0]) << " " << stringify(row[1]) << " " << stringify(row[2]);
        if (i != 2) stream << "; ";
    }
    stream << "}, t={" << translation[0] << ", " << translation[1] << ", " << translation[2] << "}";
    stream << "}";

    return stream.str();
}

std::string VoxelChunkInfo::toString() const
{
    std::stringstream stream;
    stream << "VoxelChunkInfo";
    stream << "{size=" << size.toString();
    stream << ", voxelCount=" << voxelCount;
    stream << ", pos=" << pos << "}";
    return stream.str();
}

void Reader::updateTransformForCurrentShape()
{
    const auto baseParentId = voxelChunkInfos[state.modelIndex].parentIds[state.parentIndex];
    const auto &baseParentNode = nodeMap.at(baseParentId);
    VXIO_DEBUG_ASSERT(baseParentNode.type == NodeType::TRANSFORM);

    state.transform = transformations[baseParentNode.contentId];

    // using this loop, we are working our way up the scene graph to the root node
    // we concatenate all transforms on the way to obtain a transform for the current shape
    auto parentId = baseParentId;
    for (auto iter = nodeParentMap.find(parentId); iter != nodeParentMap.end(); iter = nodeParentMap.find(parentId)) {
        parentId = iter->second;
        const auto &parentNode = nodeMap.at(parentId);
        // on our way, we might also find a GROUP node, which we don't process here
        if (parentNode.type == NodeType::TRANSFORM) {
            const auto &parentTransform = transformations.at(parentNode.contentId);
            state.transform = Transformation::concat(parentTransform, state.transform);
        }
    }
    VXIO_LOG(SPAM,
             "updated transform for current parent (" + stringify(baseParentId) + ") to " + state.transform.toString() +
                 " (" + stringify(baseParentNode.contentId) + ")");
}

[[nodiscard]] ReadResult Reader::readOneVoxel(const Vec3u32 &doublePivot) noexcept
{
    static_assert(std::numeric_limits<u8>::max() < PALETTE_SIZE);

    u8 xyzi[4];
    stream.read(xyzi, sizeof(xyzi));
    VXIO_NO_EOF();

    Vec3i32 pos = state.transform.apply(Vec3u32{xyzi[0], xyzi[1], xyzi[2]}, doublePivot);
    // swap necessary because gravity axis is z for magica and y for us
    std::swap(pos[1], pos[2]);
    pos[2] = -pos[2];

    auto rgb = palette[xyzi[3]];
    writeHelper.write(Voxel32{pos, {rgb}});
    VXIO_LOG(SUPERSPAM,
             "voxel " + pos.toString() + ", color index " + stringify(xyzi[3]) + " raw " +
                 Vec3u8(xyzi[0], xyzi[1], xyzi[2]).toString() + " i " + stringify(xyzi[3]));

    return ReadResult::ok();
}

ReadResult Reader::doRead() noexcept
{
    VXIO_DEBUG_ASSERT(initialized);
    VXIO_ASSERT(stream.good());

    const auto seekCurrentModel = [this]() -> void {
        stream.seekAbsolute(voxelChunkInfos[state.modelIndex].pos);
    };

    while (state.modelIndex < voxelChunkInfos.size()) {
        const VoxelChunkInfo &chunk = voxelChunkInfos[state.modelIndex];
        // The pivot of rotation is always on the grid between voxels in Magica.
        // Subtracting the double position from this double pivot gives us the double position rel. to the center.
        // Adding one() is important because this is a 0.5 offset in actual coordinates.
        // We must calculate with the double coordinate system, otherwise we could not represent a 0.5.
        //
        // By doing & ~1 we drop the least significant bit in double coordinates.
        // In actual coordinates, this snaps the pivot to the next lower grid vertex.
        // This is another reason why we need to use the double coordinate system.
        const Vec3u32 doublePivot =
            Vec3u32{chunk.size.x() & ~1u, chunk.size.y() & ~1u, chunk.size.z() & ~1u} - Vec3u32::one();

        while (state.parentIndex < chunk.parentIds.size()) {
            for (; state.voxelIndex < chunk.voxelCount; ++state.voxelIndex) {
                if (writeHelper.isFull()) {
                    VXIO_LOG(SPAM, "buffer is full, pausing read process");
                    return ReadResult::ok(writeHelper.voxelsWritten());
                }
                VXIO_FORWARD_ERROR(readOneVoxel(doublePivot));
            }
            state.voxelIndex = 0;
            if (++state.parentIndex < chunk.parentIds.size()) {
                LOG_CURRENT_SHAPE();
                updateTransformForCurrentShape();
                seekCurrentModel();
                VXIO_DEBUG_ASSERT(stream.good());
            }
        }
        if (++state.modelIndex < voxelChunkInfos.size()) {
            state.parentIndex = 0;
            LOG_CURRENT_VOXEL_CHUNK();
            LOG_CURRENT_SHAPE();
            updateTransformForCurrentShape();
            seekCurrentModel();
            VXIO_DEBUG_ASSERT(stream.good());
        }
    }

    return ReadResult::end(writeHelper.voxelsWritten());
}

ReadResult Reader::expectChars(const char name[CHUNK_NAME_LENGTH]) noexcept
{
    char buffer[MAGIC_LENGTH];
    stream.read(reinterpret_cast<u8 *>(buffer), MAGIC_LENGTH);
    for (size_t i = 0; i < MAGIC_LENGTH; ++i) {
        if (buffer[i] != name[i]) {
            Error error = {stream.position(), std::string{"expected magic \""} + MAGIC + '"'};
            return {0, ResultCode::READ_ERROR_UNEXPECTED_SYMBOL, std::move(error)};
        }
    }
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readString(std::string &out) noexcept
{
    u32 size = stream.readLittle<u32>();
    VXIO_NO_EOF();
    stream.readStringTo(out, static_cast<size_t>(size));
    VXIO_NO_EOF();

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readDict(std::unordered_map<std::string, std::string> &out) noexcept
{
    std::string key;
    std::string value;

    u32 size = stream.readLittle<u32>();
    VXIO_NO_EOF();
    for (size_t i = 0; i < size; ++i) {
        VXIO_FORWARD_ERROR(readString(key));
        VXIO_FORWARD_ERROR(readString(value));
        out.emplace(key, value);
    }

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::skipChunk() noexcept
{
    ChunkHeader header;
    VXIO_FORWARD_ERROR(readChunkHeader(false, header));

    stream.seekRelative(header.totalSize());
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::skipString() noexcept
{
    u32 size = stream.readLittle<u32>();
    VXIO_NO_EOF();
    stream.seekRelative(size);
    VXIO_NO_EOF();
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::skipDict() noexcept
{
    u32 size = stream.readLittle<u32>();
    VXIO_NO_EOF();
    for (size_t i = 0; i < size * 2; ++i) {
        VXIO_FORWARD_ERROR(skipString());
    }
    return ReadResult::ok();
}

ReadResult Reader::readMagicAndVersion() noexcept
{
    auto result = expectChars(MAGIC);
    if (result.type == ResultCode::READ_ERROR_UNEXPECTED_SYMBOL) {
        Error error = {stream.position(), std::string{"expected magic \""} + MAGIC + '"'};
        return {0, ResultCode::READ_ERROR_UNEXPECTED_MAGIC, std::move(error)};
    }
    else if (result.isBad()) {
        return result;
    }

    u32 version = stream.readLittle<u32>();
    VXIO_NO_EOF();
    if (version != CURRENT_VERSION) {
        return {0, ResultCode::READ_ERROR_UNKNOWN_VERSION};
    }

    return ReadResult::ok();
}

ReadResult Reader::readChunk(bool isEofAtFirstByteAllowed) noexcept
{
    ChunkHeader header;
    auto result = readChunkHeader(isEofAtFirstByteAllowed, header);
    if (result.isBad()) return result;
    if (result.type == ResultCode::READ_OBJECT_END) return ReadResult::ok();

    result = readChunkContent(header);
    state.previousChunkType = header.type;
    return result;
}

ReadResult Reader::readChunkHeader(bool isEofAtFirstByteAllowed, ChunkHeader &out) noexcept
{
    ChunkType type;
    if (auto result = readChunkType(type); result.isBad()) {
        if (isEofAtFirstByteAllowed && result.type == ResultCode::READ_ERROR_UNEXPECTED_EOF) {
            return ReadResult::nextObject();
        }
        else
            return result;
    }

    u32 selfSize = stream.readLittle<u32>();
    u32 childrenSize = stream.readLittle<u32>();
    VXIO_NO_EOF();

    const auto produceLogMessage = [=]() -> std::string {
        std::stringstream ss;
        ss << "reading " << nameOf(type);
        ss << " (12head + ";
        ss << stringify(selfSize) << "self + ";
        ss << stringify(childrenSize) << "children = ";
        ss << stringify(12 + selfSize + childrenSize);
        ss << ") @";
        ss << (this->stream.position() - 12);
        return ss.str();
    };
    VXIO_LOG(SPAM, produceLogMessage());

    out = {type, selfSize, childrenSize};
    return ReadResult::ok();
}

ReadResult Reader::readChunkType(ChunkType &out) noexcept
{
    u32 id = stream.readBig<u32>();
    if (stream.eof()) {
        return ReadResult::unexpectedEof(stream.position());
    }
    if (not isValidChunkType(id)) {
        Error error = {stream.position(), "invalid chunk id: 0x" + stringifyHex(id)};
        return {0, ResultCode::READ_ERROR_CORRUPTED_ENUM, std::move(error)};
    }
    out = static_cast<ChunkType>(id);
    return ReadResult::ok();
}

ReadResult Reader::readChunkContent(const ChunkHeader &header) noexcept
{
    switch (header.type) {
    case ChunkType::PACK: {
        Error error = {stream.position(), "PACK chunks are not supported"};
        return {0, ResultCode::READ_ERROR_UNSUPPORTED_FEATURE, std::move(error)};
    }

    // we don't need materials or renderer settings
    case ChunkType::MATL:
    case ChunkType::MATT:
    case ChunkType::IMAP:
    case ChunkType::rOBJ: stream.seekRelative(header.totalSize()); return ReadResult::ok();

    case ChunkType::MAIN: return readChunkContent_main();
    case ChunkType::SIZE: return readChunkContent_size();
    case ChunkType::XYZI: return readChunkContent_xyzi();
    case ChunkType::RGBA: return readChunkContent_rgba();
    case ChunkType::nTRN: return readChunkContent_nodeTransform();
    case ChunkType::nGRP: return readChunkContent_nodeGroup();
    case ChunkType::nSHP: return readChunkContent_nodeShape();
    case ChunkType::LAYR: return readChunkContent_layer();
    }

    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

ReadResult Reader::readChunkContent_main() noexcept
{
    if (initialized) {
        Error error = {stream.position(), "multiple main chunks found"};
        return {0, ResultCode::READ_ERROR_MULTIPLE_ROOTS, error};
    }

    while (true) {
        ChunkHeader header;
        VXIO_FORWARD_ERROR(readChunkHeader(true, header));
        if (header.type != ChunkType::SIZE) {
            VXIO_LOG(SPAM, "No longer skipping because found " + std::string{nameOf(header.type)});
            VXIO_FORWARD_ERROR(readChunkContent(header));
            break;
        }
        VXIO_FORWARD_ERROR(readChunkContent_size());

        VXIO_FORWARD_ERROR(readChunkHeader(false, header));
        if (header.type != ChunkType::XYZI) {
            return {0,
                    ResultCode::READ_ERROR_UNEXPECTED_SYMBOL,
                    {{stream.position(),
                      std::string{"Expected SIZE chunk to be followed by XYZI, but got "} + nameOf(header.type)}}};
        }
        VXIO_ASSERT(not voxelChunkInfos.empty());
        auto &voxelChunk = voxelChunkInfos.back();
        voxelChunk.voxelCount = stream.readLittle<u32>();
        voxelChunk.pos = stream.position();
        VXIO_NO_EOF();
        VXIO_ASSERT(voxelChunk.pos != u64(-1));
        VXIO_LOG(SPAM, "Memorizing " + voxelChunk.toString() + " for 2nd pass");

        stream.seekRelative(header.totalSize() - sizeof(u32));
    }

    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_rgba() noexcept
{
    for (size_t i = 0; i < PALETTE_SIZE; ++i) {
        u32 rgba = stream.readBig<u32>();
        VXIO_NO_EOF();
        this->palette[(i + 1) % PALETTE_SIZE] = reorderColor<RGBA, ARGB>(rgba);
    }
    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_size() noexcept
{
    Vec3u32 size;
    stream.readLittle<3>(size.data());
    VXIO_NO_EOF();
    this->voxelChunkInfos.push_back({size, 0, u64(-1)});
    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_xyzi() noexcept
{
    return ReadResult::ok();
}

ReadResult Reader::makeError_expectedButGot(const std::string &field, i64 expected, i64 actual) noexcept
{
    std::stringstream messageStream;
    messageStream << "Expected " << field << " to be " << expected << " but got " << actual;
    return {0, ResultCode::READ_ERROR_UNEXPECTED_SYMBOL, {{stream.position(), messageStream.str()}}};
}

ReadResult Reader::readChunkContent_nodeTransform() noexcept
{
    u32 nodeId = stream.readLittle<u32>();
    VXIO_NO_EOF();
    VXIO_FORWARD_ERROR(skipDict());  // attributes, unused
    u32 childNodeId = stream.readLittle<u32>();
    i32 reservedId = stream.readLittle<i32>();
    stream.seekRelative(sizeof(i32));  // layerId, unused
    u32 numOfFrames = stream.readLittle<u32>();
    VXIO_NO_EOF();

    if (reservedId != -1) return makeError_expectedButGot("reservedId", -1, reservedId);
    if (numOfFrames != 1) return makeError_expectedButGot("numOfFrames", 1, numOfFrames);

    Transformation transform;
    VXIO_FORWARD_ERROR(readTransformationDict(transform));
    u32 transformId = static_cast<u32>(transformations.size());
    transformations.push_back(std::move(transform));
    VXIO_LOG(SPAM,
             "decoded transform " + transform.toString() + " for node " + stringify(nodeId) + " as transform " +
                 stringify(transformId));

    u32 parentNodeId;
    auto parentIter = nodeParentMap.find(nodeId);
    if (parentIter == nodeParentMap.end()) {
        this->rootNodeId = parentNodeId = nodeId;
    }
    else {
        parentNodeId = parentIter->second;
    }
    nodeMap.emplace(nodeId, SceneNode{NodeType::TRANSFORM, transformId});
    nodeParentMap.emplace(childNodeId, nodeId);

    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_nodeGroup() noexcept
{
    u32 nodeId = stream.readLittle<u32>();
    VXIO_NO_EOF();
    VXIO_FORWARD_ERROR(skipDict());  // attributes, unused
    u32 numOfChildNodes = stream.readLittle<u32>();
    VXIO_NO_EOF();

    std::vector<u32> children;
    for (size_t i = 0; i < numOfChildNodes; ++i) {
        children.push_back(stream.readLittle<u32>());
    }
    VXIO_NO_EOF();

    if (auto parentIter = nodeParentMap.find(nodeId); parentIter == nodeParentMap.end()) {
        return ReadResult::parseError(stream.position(), "nGRP without parent found");
    }

    nodeMap.emplace(nodeId, SceneNode{NodeType::GROUP, 0});
    for (u32 childNodeId : children) {
        nodeParentMap.emplace(childNodeId, nodeId);
    }

    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_nodeShape() noexcept
{
    u32 nodeId = stream.readLittle<u32>();
    VXIO_FORWARD_ERROR(skipDict());  // attributes, unused
    u32 numOfModels = stream.readLittle<u32>();
    VXIO_NO_EOF();
    if (numOfModels != 1) {
        return makeError_expectedButGot("numOfModels", 1, numOfModels);
    }

    // for every model, but there is only one, so we don't loop
    u32 modelId = stream.readLittle<u32>();
    VXIO_NO_EOF();
    if (modelId >= voxelChunkInfos.size()) {
        return ReadResult::parseError(stream.position(), "modelId " + stringify(modelId) + " out of range");
    }

    VXIO_FORWARD_ERROR(skipDict());  // this dict is reserved

    auto [parentsBegin, parentsEnd] = nodeParentMap.equal_range(nodeId);
    if (parentsBegin == parentsEnd) {
        return ReadResult::parseError(stream.position(), "nSHP without parents found");
    }

    const auto &[iter, success] = nodeMap.emplace(nodeId, SceneNode{NodeType::SHAPE, modelId});
    VXIO_DEBUG_ASSERT(success);
    shapeNodeIds.push_back(nodeId);

    return ReadResult::ok();
}

ReadResult Reader::readChunkContent_layer() noexcept
{
    stream.seekRelative(sizeof(u32));  // u32 layerId, unused
    VXIO_NO_EOF();
    VXIO_FORWARD_ERROR(skipDict());  // attributes, unused
    i32 reservedId = stream.readLittle<i32>();
    if (reservedId != -1) {
        return makeError_expectedButGot("reservedId", -1, reservedId);
    }
    return ReadResult::ok();
}

ReadResult Reader::decodeRotation(u8 bits, Transformation &out) noexcept
{
    constexpr uint32_t row2IndexTable[] = {UINT32_MAX, UINT32_MAX, UINT32_MAX, 2, UINT32_MAX, 1, 0, UINT32_MAX};

    // compute the per-row indexes into k_vectors[] array.
    // unpack rotation bits.
    //  bits  : meaning
    //  0 - 1 : index of the non-zero entry in the first row
    //  2 - 3 : index of the non-zero entry in the second row
    // index in last row the needs to be in the remaining column, chosen via lookup table
    //  4 - 6 : sign bits of rows, 0 is positive
    u32 indicesOfOnesPerRow[3]{(bits >> 0) & 0b11u,
                               (bits >> 2) & 0b11u,
                               row2IndexTable[(1 << indicesOfOnesPerRow[0]) | (1 << indicesOfOnesPerRow[1])]};
    if (indicesOfOnesPerRow[2] == UINT32_MAX) {
        return ReadResult::unexpectedSymbol(stream.position(), "invalid rotation: 0b" + stringifyBin(bits));
    }

    for (size_t i = 0; i < 3; ++i) {
        const u8 sign = (bits >> (i + 4)) & 1;
        const auto indexOfOne = indicesOfOnesPerRow[i];
        out.matrix[i][indexOfOne] = 1 - static_cast<i8>(2 * sign);
        out.matrix[i][(indexOfOne + 1) % 3] = 0;
        out.matrix[i][(indexOfOne + 2) % 3] = 0;
    }

    return ReadResult::ok();
}

[[nodiscard]] static ReadResult parseTranslation(u64 pos, const std::string &str, Vec3i32 &out)
{
    auto splits = splitAtDelimiter(str, ' ', 3);
    if (splits.size() != 3) {
        Error error = {
            pos, "Expected value of " + std::string{KEY_ROTATION} + " to be 3 space-separated integers, got " + str};
        return {0, ResultCode::READ_ERROR_ILLEGAL_DATA_LENGTH, error};
    }

    for (size_t i = 0; i < 3; ++i) {
        if (not parse(splits[i], out[i])) {
            Error error = {
                pos, "Failed to parse translation integer " + splits[i] + " at index " + stringify(i) + " in " + str};
            return {0, ResultCode::READ_ERROR_TEXT_DATA_PARSE_FAIL, error};
        }
    }

    return ReadResult::ok();
}

ReadResult Reader::readTransformationDict(Transformation &out) noexcept
{
    dict_t dict;
    VXIO_FORWARD_ERROR(readDict(dict));

    if (auto iter = dict.find(KEY_ROTATION); iter != dict.end()) {
        const auto &str = iter->second;
        u8 bits;
        if (not parse(str, bits)) {
            Error error = {stream.position(), "Failed to parse rotation integer \"" + str + '"'};
            return {0, ResultCode::READ_ERROR_TEXT_DATA_PARSE_FAIL, error};
        }
        VXIO_FORWARD_ERROR(decodeRotation(bits, out));
    }
    else {
        out.matrix[0] = {i8{1}, i8{0}, i8{0}};
        out.matrix[1] = {i8{0}, i8{1}, i8{0}};
        out.matrix[2] = {i8{0}, i8{0}, i8{1}};
    }

    if (auto iter = dict.find(KEY_TRANSLATION); iter != dict.end()) {
        VXIO_FORWARD_ERROR(parseTranslation(stream.position(), iter->second, out.translation));
    }

    return ReadResult::ok();
}

}  // namespace voxelio::vox
