#include "qb.hpp"

#include "log.hpp"
#include "macro.hpp"
#include "stringify.hpp"

#include <stdexcept>

namespace voxelio {
namespace qb {

Matrix::Matrix(std::string name_, Vec3i32 pos_, VoxelArray voxels_)
    : MatrixHeader{std::move(name_), pos_, voxels_.dimensions().cast<u32>()}, voxels{std::move(voxels_)}
{
}

size_t Model::computeCombinedVolume() const
{
    size_t v = 0;
    for (const Matrix &e : _matrices)
        v += e.voxels.volume();
    return v;
}

size_t Model::countVoxels() const
{
    size_t count = 0;
    for (const Matrix &e : _matrices)
        count += e.voxels.countVoxels();
    return count;
}

std::pair<Vec3i32, Vec3i32> Model::findBoundaries() const
{
    if (empty()) throw std::length_error("empty meshes have no boundaries");
    i32 result[6] = {INT32_MAX, INT32_MAX, INT32_MAX, 0, 0, 0};

    for (const Matrix &matrix : _matrices) {
        auto [min, max] = matrix.bounds();

        if (min.x() < result[0]) result[0] = min.x();
        if (min.y() < result[1]) result[1] = min.y();
        if (min.z() < result[2]) result[2] = min.z();
        if (max.x() > result[3]) result[3] = max.x();
        if (max.y() > result[4]) result[4] = max.y();
        if (max.z() > result[5]) result[5] = max.z();
    }

    return {{result[0], result[1], result[2]}, {result[3], result[4], result[5]}};
}

Color32 decodeColor(argb32 color, ColorFormat colorFormat, bool visibilityMaskEncoded)
{
    VXIO_DEBUG_ASSERT(colorFormat == ColorFormat::RGBA || colorFormat == ColorFormat::BGRA);

    switch (colorFormat) {
    case ColorFormat::RGBA: color = reorderColor<RGBA, ARGB>(color); break;
    case ColorFormat::BGRA: color = reorderColor<BGRA, ARGB>(color); break;
    }

    // DEBUG_ASSERT_CONSEQUENCE(visibilityMaskEncoded, RGB32::alpha(color) < 0b1000000);

    // if any side is visible, make color solid
    if (visibilityMaskEncoded && Color32(color).isVisible()) {
        color |= 0xFF000000;
    }

    return color;
}

template <ColorFormat FORMAT>
u32 encodeColor(Color32 argb)
{
    static_assert(FORMAT == ColorFormat::RGBA || FORMAT == ColorFormat::BGRA);

    if constexpr (FORMAT == ColorFormat::RGBA) return reorderColor<ARGB, RGBA>(argb);
    if constexpr (FORMAT == ColorFormat::BGRA) return reorderColor<ARGB, BGRA>(argb);
}

ReadResult Reader::init() noexcept
{
    initialized = true;
    VXIO_FORWARD_ERROR(deserializeHeader());
    if (header.numMatrices != 0) {
        VXIO_FORWARD_ERROR(deserializeMatrixHeader());
        return ReadResult::nextObject();
    }
    else {
        return ReadResult::end();
    }
}

ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    VXIO_ASSERT_NOTNULL(buffer);
    VXIO_ASSERT_NE(bufferLength, 0);

    if (not initialized) {
        // we must return without reading any voxels so that the user gets an opportunity to handle NEXT_OBJECT
        // if we kept reading here, the reader may hit the next object and the information about the first one
        // would be lost
        //
        // the user can avoid this behavior by explicitly calling init() and handling the result there
        return init();
    }

    // will be either OK or NEXT_OBJECT (or bad)
    auto result = header.compressed ? readCompressed(buffer, bufferLength) : readUncompressed(buffer, bufferLength);

    if (result.type == ResultCode::READ_OBJECT_END) {
        if (++matrixIndex < header.numMatrices) {
            auto subResult = deserializeMatrixHeader();
            if (subResult.isBad()) {
                result.type = subResult.type;
            }
        }
        else {
            result.type = ResultCode::READ_END;
        }
    }

    return result;
}

ReadResult Reader::deserializeHeader_version() noexcept
{
    auto version = stream.readBig<QBVersion>();
    VXIO_NO_EOF();
    if (version != QBVersion::CURRENT) {
        return ReadResult::unknownVersion(stream.position(),
                                          stringifyHex(static_cast<u32>(version)) + " != current (" +
                                              stringifyHex(static_cast<u32>(QBVersion::CURRENT)) + ")");
    }
    return ReadResult::ok();
}

ReadResult Reader::deserializeHeader_colorFormat() noexcept
{
    header.colorFormat = stream.readBig<ColorFormat>();
    VXIO_NO_EOF();
    if (header.colorFormat != ColorFormat::RGBA && header.colorFormat != ColorFormat::BGRA) {
        return ReadResult::unknownFeature(stream.position(), "unknown color format: " + stringify(header.colorFormat));
    }
    return ReadResult::ok();
}

ReadResult Reader::deserializeHeader_zOrient() noexcept
{
    auto zAxisOrientation = stream.readBig<ZOrient>();
    VXIO_NO_EOF();
    if (zAxisOrientation != ZOrient::LEFT && zAxisOrientation != ZOrient::RIGHT) {
        return ReadResult::unknownFeature(stream.position(),
                                          "unknown z axis orientation: " + stringify(zAxisOrientation));
    }
    header.zLeft = zAxisOrientation == ZOrient::LEFT;
    return ReadResult::ok();
}

ReadResult Reader::deserializeHeader_compression() noexcept
{
    auto compressedInt = stream.readLittle<Compressed>();
    VXIO_NO_EOF();
    if (compressedInt != Compressed::FALSE && compressedInt != Compressed::TRUE) {
        return ReadResult::unknownFeature(stream.position(), "unknown compression: " + stringify(compressedInt));
    }
    header.compressed = compressedInt == Compressed::TRUE;
    return ReadResult::ok();
}

ReadResult Reader::deserializeHeader_visMaskEncoded() noexcept
{
    auto visEncodedInt = stream.readLittle<VisMaskEncoded>();
    VXIO_NO_EOF();
    if (visEncodedInt != VisMaskEncoded::FALSE && visEncodedInt != VisMaskEncoded::TRUE) {
        return ReadResult::unknownFeature(stream.position(), "unknown vis mask encoding: " + stringify(visEncodedInt));
    }
    header.visibilityMaskEncoded = visEncodedInt == VisMaskEncoded::TRUE;
    return ReadResult::ok();
}

ReadResult Reader::deserializeHeader() noexcept
{
    VXIO_FORWARD_ERROR(deserializeHeader_version());
    VXIO_FORWARD_ERROR(deserializeHeader_colorFormat());
    VXIO_FORWARD_ERROR(deserializeHeader_zOrient());
    VXIO_FORWARD_ERROR(deserializeHeader_compression());
    VXIO_FORWARD_ERROR(deserializeHeader_visMaskEncoded());
    header.numMatrices = stream.readLittle<u32>();
    VXIO_NO_EOF();

    VXIO_LOG(DEBUG,
             "deserializing " + stringify(header.numMatrices) + " matrices with" +
                 ": compression=" + stringify(header.compressed) + ", colorFormat=" + stringify(header.colorFormat) +
                 ", visMaskEncoded=" + stringify(header.visibilityMaskEncoded) + ", zLeft=" + stringify(header.zLeft));

    return ReadResult::ok();
}

ReadResult Reader::deserializeMatrixHeaderName() noexcept
{
    const size_t nameLength = stream.readU8();
    VXIO_NO_EOF();
    stream.readStringTo(matrixName, nameLength);
    VXIO_NO_EOF();

    return ReadResult::ok();
}

ReadResult Reader::deserializeMatrixHeader() noexcept
{
    VXIO_FORWARD_ERROR(deserializeMatrixHeaderName());

    matSizeX = stream.readLittle<u32>();
    matSizeY = stream.readLittle<u32>();
    matSizeZ = stream.readLittle<u32>();
    matPosX = stream.readLittle<i32>();
    matPosY = stream.readLittle<i32>();
    matPosZ = stream.readLittle<i32>();
    VXIO_NO_EOF();
    x = y = slice = index = 0;
    matVolume = matSizeX * matSizeY * matSizeZ;

    VXIO_LOG(DEBUG,
             "reading matrix '" + matrixName + "' : " + stringify(matSizeX) + "x" + stringify(matSizeY) + "x" +
                 stringify(matSizeZ) + " at " + stringify(matPosX) + ", " + stringify(matPosY) + ", " +
                 stringify(matPosZ));
    return ReadResult::ok();
}

ReadResult Reader::readUncompressed(Voxel64 buffer[], size_t bufferLength) noexcept
{
    VXIO_DEBUG_ASSERT(this->initialized);
    VXIO_DEBUG_ASSERT_LE(index, matVolume);
    VXIO_DEBUG_ASSERT_GT(bufferLength, 0);

    const auto voxelsLeftInMatrixCount = matVolume - index;
    if (voxelsLeftInMatrixCount == 0) return ReadResult::nextObject();

    const auto tmpBufferSize = std::min(bufferLength, voxelsLeftInMatrixCount);
    const auto tmpBuffer = std::make_unique<u8[]>(tmpBufferSize * sizeof(u32));
    const auto *tmpBufferPtr = reinterpret_cast<u8 *>(tmpBuffer.get());

    stream.read(tmpBuffer.get(), tmpBufferSize * sizeof(u32));
    if (tmpBufferSize < voxelsLeftInMatrixCount) {
        VXIO_NO_EOF();
    }

    static_assert(sizeof(tmpBuffer[0]) == sizeof(u8));

    u64 voxelsRead = 0;
    size_t tmpIndex = 0;
    for (; slice < matSizeZ; slice++) {
        const u32 z = header.zLeft ? slice : matSizeZ - slice - 1;
        for (; y < matSizeY; y++) {
            for (; x < matSizeX; x++, index++, tmpIndex++) {
                if (index == matVolume) {
                    return ReadResult::nextObject(voxelsRead);
                }
                else if (tmpIndex == tmpBufferSize) {
                    return ReadResult::ok(voxelsRead);
                }

                VXIO_DEBUG_ASSERT_LT(tmpIndex, tmpBufferSize);
                const u32 data = decodeBig<u32>(tmpBufferPtr + tmpIndex * sizeof(u32));
                const Color32 color = decodeColor(data, header.colorFormat, header.visibilityMaskEncoded);
                if (color.isVisible()) {
                    const auto pos = Vec3i64{matPosX, matPosY, matPosZ} + Vec3u32{x, y, z};
                    buffer[voxelsRead++] = {std::move(pos), {color}};
                }
            }
            x = 0;
        }
        y = 0;
    }

    return ReadResult::nextObject(voxelsRead);
}

std::pair<u32, u32> Reader::readCompressed_writeToBuffer(
    Voxel64 buffer[], size_t bufferLength, size_t bufferIndex, size_t relZ, u32 data, size_t count) noexcept
{
    const Color32 color = decodeColor(data, this->header.colorFormat, this->header.visibilityMaskEncoded);
    if (color.isInvisible()) {
        this->index += count;
        return {0, count};
    }

    VXIO_IF_DEBUG(auto oldIndex = this->index);
    size_t lim = std::min(count, bufferLength - bufferIndex);
    for (size_t i = 0; i < lim; ++i, ++this->index) {
        const u64 relIndex = this->index % (matSizeX * matSizeY);
        const u64 relX = relIndex % matSizeX;
        const u64 relY = relIndex / matSizeX;
        const Vec3i64 pos = Vec3u64{relX, relY, relZ}.cast<i64>();

        VXIO_DEBUG_ASSERT_LT(bufferIndex, bufferLength);
        VXIO_DEBUG_ASSERT_LT(this->index, matVolume);
        VXIO_DEBUG_ASSERT_LT(relX, this->matSizeX);
        VXIO_DEBUG_ASSERT_LT(relY, this->matSizeY);
        VXIO_DEBUG_ASSERT_LT(relZ, this->matSizeZ);

        buffer[bufferIndex + i] = {pos + Vec3i64{matPosX, matPosY, matPosZ}, {color}};
    }

    VXIO_DEBUG_ASSERT_EQ(index, oldIndex + lim);
    return {lim, lim};
}

ReadResult Reader::readCompressed(Voxel64 buffer[], size_t bufferLength) noexcept
{
    VXIO_DEBUG_ASSERT(this->initialized);
    VXIO_DEBUG_ASSERT_LE(index, matVolume);
    VXIO_DEBUG_ASSERT_GT(bufferLength, 0);
    VXIO_IF_DEBUG(auto oldIndex = index);

    u32 readVoxels = 0;

    const auto writeToBuffer = [this, &readVoxels, buffer, bufferLength](size_t z, u32 data, size_t count) {
        return readCompressed_writeToBuffer(buffer, bufferLength, readVoxels, z, data, count);
    };

    u32 &resumeCount = this->x;
    u32 &resumeData = this->y;

    if (resumeCount != 0) {
        const auto z = header.zLeft ? slice : matSizeZ - size_t{1} - slice;
        const auto [actualCount, actualVolume] = writeToBuffer(z, resumeData, resumeCount);
        readVoxels += actualCount;
        VXIO_DEBUG_ASSERT_NE(actualVolume, 0);
        VXIO_DEBUG_ASSERT_GE(resumeCount, actualVolume);
        resumeCount -= actualVolume;

        if (resumeCount != 0 || readVoxels == bufferLength) {
            VXIO_DEBUG_ASSERT_GT(index, oldIndex);
            VXIO_DEBUG_ASSERT_NE(readVoxels, 0);
            return ReadResult::ok(readVoxels);
        }
    }
    VXIO_DEBUG_ASSERT_EQ(resumeCount, 0);

    for (; slice < matSizeZ; ++slice) {
        const auto z = header.zLeft ? slice : matSizeZ - size_t{1} - slice;

        while (true) {
            const auto data = stream.readLittle<u32>();
            VXIO_NO_EOF();
            if (static_cast<CompressionFlags>(data) == CompressionFlags::NEXTSLICEFLAG) {
                break;
            }
            else if (static_cast<CompressionFlags>(data) == CompressionFlags::CODEFLAG) {
                const auto count = stream.readLittle<u32>();
                const auto colorData = stream.readBig<u32>();
                VXIO_NO_EOF();
                const auto [actualCount, actualVolume] = writeToBuffer(z, colorData, count);
                readVoxels += actualCount;
                if (readVoxels == bufferLength) {
                    resumeCount = static_cast<u32>(count - actualVolume);
                    resumeData = colorData;
                    VXIO_DEBUG_ASSERT_GT(index, oldIndex);
                    VXIO_DEBUG_ASSERT_NE(actualCount, 0);
                    VXIO_DEBUG_ASSERT_NE(readVoxels, 0);
                    return ReadResult::ok(readVoxels);
                }
            }
            else {
                const auto colorData = reverseBytes<u32>(data);
                const auto [actualCount, actualVolume] = writeToBuffer(z, colorData, 1);
                readVoxels += actualCount;
                if (readVoxels == bufferLength) {
                    VXIO_DEBUG_ASSERT_GT(index, oldIndex);
                    VXIO_DEBUG_ASSERT_NE(actualCount, 0);
                    VXIO_DEBUG_ASSERT_NE(readVoxels, 0);
                    return ReadResult::ok(readVoxels);
                }
            }
        }
    }

    VXIO_DEBUG_ASSERT_EQ(this->index, this->matVolume);
    return ReadResult::nextObject(readVoxels);
}

Model Deserializer::read() noexcept(false)
{
    VXIO_LOG(DEBUG, "Deserializing QB model ...");

    deserializeHeader();

    Model mesh;
    for (u32 i = 0; i < header.numMatrices; i++)
        deserializeMatrix(mesh);

    VXIO_LOG(DEBUG, "Deserialized all matrices");

    return mesh;
}

void Deserializer::deserializeHeader() noexcept(false)
{
    auto version = streamWrapper.readBig<QBVersion>();  // big endian
    if (version != QBVersion::CURRENT)
        throw std::runtime_error(stringifyHex(static_cast<u32>(version)) + " != current (" +
                                 stringifyHex(static_cast<u32>(QBVersion::CURRENT)) + ")");

    header.colorFormat = streamWrapper.readBig<ColorFormat>();
    if (header.colorFormat != ColorFormat::RGBA && header.colorFormat != ColorFormat::BGRA)
        throw std::runtime_error("unknown color format: " + stringify(header.colorFormat));

    auto zAxisOrientation = streamWrapper.readBig<ZOrient>();
    if (zAxisOrientation != ZOrient::LEFT && zAxisOrientation != ZOrient::RIGHT)
        throw std::runtime_error("unknown z axis orientation: " + stringify(zAxisOrientation));
    header.zLeft = zAxisOrientation == ZOrient::LEFT;

    auto compressedInt = streamWrapper.readLittle<Compressed>();
    if (compressedInt != Compressed::FALSE && compressedInt != Compressed::TRUE)
        throw std::runtime_error("unknown compression: " + stringify(compressedInt));
    header.compressed = compressedInt == Compressed::TRUE;

    auto visEncodedInt = streamWrapper.readLittle<VisMaskEncoded>();
    if (visEncodedInt != VisMaskEncoded::FALSE && visEncodedInt != VisMaskEncoded::TRUE)
        throw std::runtime_error("unknown vis mask encoding: " + stringify(visEncodedInt));
    header.visibilityMaskEncoded = visEncodedInt == VisMaskEncoded::TRUE;

    header.numMatrices = streamWrapper.readLittle<u32>();

    VXIO_LOG(DEBUG,
             "deserializing " + stringify(header.numMatrices) + " matrices with" +
                 ": compression=" + stringify(header.compressed) + ", colorFormat=" + stringify(header.colorFormat) +
                 ", visMaskEncoded=" + stringify(header.visibilityMaskEncoded) + ", zLeft=" + stringify(header.zLeft));
}

void Deserializer::deserializeMatrix(Model &mesh) noexcept(false)
{
    // read matrix name
    const u8 nameLength = streamWrapper.readU8();
    std::string name = streamWrapper.readString(nameLength);
    if (not streamWrapper.good()) {
        throw std::runtime_error("I/O error");
    }

    u32 size[3];
    streamWrapper.readLittle<3>(size);
    i32 pos[3];
    streamWrapper.readLittle<3>(pos);

    VXIO_LOG(DEBUG,
             "reading matrix '" + name + "'<-length " + stringify(nameLength) + ": " + stringify(size[0]) + "x" +
                 stringify(size[1]) + "x" + stringify(size[2]) + " at " + stringify(pos[0]) + ", " + stringify(pos[1]) +
                 ", " + stringify(pos[2]));

    VoxelArray voxels =
        header.compressed ? readCompressed(size[0], size[1], size[2]) : readUncompressed(size[0], size[1], size[2]);

    mesh.matrices().emplace_back(std::move(name), Vec3i32{pos[0], pos[1], pos[2]}, std::move(voxels));
}

VoxelArray Deserializer::readUncompressed(u32 sizeX, u32 sizeY, u32 sizeZ) noexcept(false)
{
    VoxelArray matrix{sizeX, sizeY, sizeZ};

    const size_t bufferSize = sizeX * sizeY * sizeZ * sizeof(u32);
    auto buffer = std::make_unique<u8[]>(bufferSize);
    u8 *bufferPtr = buffer.get();

    streamWrapper.read(bufferPtr, bufferSize);

    const u32 maxZ = sizeZ - 1;
    for (size_t slice = 0, index = 0; slice < sizeZ; slice++) {
        const size_t z = header.zLeft ? slice : maxZ - slice;
        for (size_t y = 0; y < sizeY; y++)
            for (size_t x = 0; x < sizeX; x++, index++) {
                auto data = decodeBig<u32>(bufferPtr + index * sizeof(u32));
                auto color = decodeColor(data, header.colorFormat, header.visibilityMaskEncoded);
                matrix[{x, y, z}] = color;
            }
    }

    return matrix;
}

VoxelArray Deserializer::readCompressed(u32 sizeX, u32 sizeY, u32 sizeZ) noexcept(false)
{
    VoxelArray voxels{sizeX, sizeY, sizeZ};
    const u32 maxZ = sizeZ - 1;

    for (size_t slice = 0; slice < sizeZ; slice++) {
        const size_t z = header.zLeft ? slice : maxZ - slice;
        size_t index = 0;

        while (true) {
            auto data = streamWrapper.readLittle<u32>();
            if (static_cast<CompressionFlags>(data) == CompressionFlags::NEXTSLICEFLAG)
                break;

            else if (static_cast<CompressionFlags>(data) == CompressionFlags::CODEFLAG) {
                auto count = streamWrapper.readLittle<u32>();
                data = streamWrapper.readBig<u32>();

                for (u32 i = 0; i < count; i++) {
                    size_t x = index % sizeX, y = index / sizeX;
                    auto color = decodeColor(data, header.colorFormat, header.visibilityMaskEncoded);
                    voxels[{x, y, z}] = color;
                    index++;
                }
            }
            else {
                size_t x = index % sizeX, y = index / sizeX;
                auto color = decodeColor(reverseBytes<u32>(data), header.colorFormat, header.visibilityMaskEncoded);
                voxels[{x, y, z}] = color;
                index++;
            }
        }
    }

    return voxels;
}

ResultCode Serializer::write(const Model &mesh) noexcept(false)
{
    serializeHeader(static_cast<u32>(mesh.matrixCount()));
    for (auto &matrix : mesh)
        serializeMatrix(matrix);
    if (not streamWrapper.good()) {
        err = {streamWrapper.position(), "Stream was not left in a good() state after writing file"};
        return ResultCode::WRITE_ERROR_IO_FAIL;
    }
    return ResultCode::OK;
}

void Serializer::serializeHeader(u32 modelSize) noexcept(false)
{
    VXIO_LOG(DEBUG, "serializing header ...");
    streamWrapper.writeBig(QBVersion::CURRENT);
    streamWrapper.writeBig(ColorFormat::RGBA);
    streamWrapper.writeBig(ZOrient::LEFT);
    streamWrapper.writeLittle(Compressed::FALSE);
    streamWrapper.writeLittle(VisMaskEncoded::FALSE);
    streamWrapper.writeLittle(modelSize);
}

void Serializer::serializeMatrix(const Matrix &matrix) noexcept(false)
{
    VXIO_LOG(DEBUG, "serializing matrix " + matrix.toString() + " ...");
    auto nameLength = static_cast<u8>(matrix.name.size());
    streamWrapper.writeU8(nameLength);
    streamWrapper.writeString(matrix.name);

    streamWrapper.writeLittle<3, u32>(matrix.size.data());
    streamWrapper.writeLittle<3, i32>(matrix.pos.data());

    serializeUncompressed(matrix.voxels);
}

void Serializer::serializeUncompressed(const VoxelArray &array) noexcept(false)
{
    const auto dims = array.dimensions();

    for (size_t z = 0; z < dims.z(); z++)
        for (size_t y = 0; y < dims.y(); y++)
            for (size_t x = 0; x < dims.x(); x++) {
                argb32 color = encodeColor<ColorFormat::RGBA>(array[{x, y, z}]);
                streamWrapper.writeBig<u32>(color);
            }
}

}  // namespace qb
}  // namespace voxelio
