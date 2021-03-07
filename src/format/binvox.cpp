#include "voxelio/format/binvox.hpp"

#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"
#include "voxelio/parse.hpp"
#include "voxelio/stringmanip.hpp"

namespace voxelio::binvox {

namespace {

constexpr const char *MAGIC = "#binvox";
constexpr u32 VERSION = 1;

}  // namespace

ReadResult Reader::init() noexcept
{
    initialized = true;
    VXIO_FORWARD_ERROR(readMagicAndVersion());
    VXIO_FORWARD_ERROR(readHeaderFields());

    if (not header.dimInitialized) {
        return ReadResult::missingHeaderField(state.lineNum, "missing dimensions in header");
    }

    Vec3u64 dim = header.dim.cast<u64>();
    header.volume = dim.x() * dim.y() * dim.z();
    if (header.volume == 0) {
        return ReadResult::end();
    }

    return ReadResult::ok();
}

ReadResult Reader::readMagicAndVersion()
{
    std::string line = stream.readStringUntil(' ');

    if (line != MAGIC) {
        return ReadResult::unexpectedMagic(state.lineNum, "expected \"#binvox\", got \"" + line + "\"");
    }

    stream.readLineTo(line);
    u32 version;
    if (not parse(line, version)) {
        return ReadResult::parseError(state.lineNum, "Failed to parse version number \"" + line + "\"");
    }
    if (version != VERSION) {
        return ReadResult::unknownVersion(state.lineNum, stringify(version));
    }

    state.lineNum++;
    return ReadResult::ok();
}

ReadResult Reader::readHeaderFields()
{
    std::string headerLine;
    bool done = 0;
    while (stream.good() && not done) {
        stream.readLineTo(headerLine);
        VXIO_NO_EOF();
        state.lineNum++;

        auto result = parseHeaderLine(headerLine);
        if (result.isBad()) return result;
        if (result.type != ResultCode::READ_OBJECT_END) break;
    }
    return ReadResult::ok();
}

#define VXIO_PARSE_SAFELY(string, target)                                                   \
    if (not::voxelio::parse(string, target)) {                                              \
        return ReadResult::parseError(state.lineNum, "Failed to parse \"" + string + "\""); \
    }

ReadResult Reader::parseHeaderLine(const std::string &line)
{
    std::vector<std::string> parts = voxelio::splitAtDelimiter(line, ' ');
    if (parts.empty()) {
        return ReadResult::parseError(state.lineNum, "Empty header line");
    }
    std::string &keyword = parts.front();

    if (keyword == "data") {
        VXIO_LOG(SPAM, "reading data ...");
        return ReadResult::ok();
    }
    if (keyword == "dim") {
        VXIO_PARSE_SAFELY(parts[1], header.dim.x());
        VXIO_PARSE_SAFELY(parts[2], header.dim.y());
        VXIO_PARSE_SAFELY(parts[3], header.dim.z());
        header.dimInitialized = true;
        VXIO_LOG(SPAM, "read dim " + header.dim.toString());
        return ReadResult::nextObject();
    }
    if (keyword == "translate") {
        VXIO_PARSE_SAFELY(parts[1], header.translation.x());
        VXIO_PARSE_SAFELY(parts[2], header.translation.y());
        VXIO_PARSE_SAFELY(parts[3], header.translation.z());
        header.translationInitialized = true;
        VXIO_LOG(SPAM, "read translate " + header.translation.toString());
        return ReadResult::nextObject();
    }
    if (keyword == "scale") {
        VXIO_PARSE_SAFELY(parts[1], header.scale);
        header.scaleInitialized = true;
        VXIO_LOG(SPAM, "read scale " + stringify(header.scale));
        return ReadResult::nextObject();
    }

    return ReadResult::unexpectedSymbol(state.lineNum, "header keyword \"" + keyword + "\"");
}

ReadResult Reader::read(Voxel64 buffer[], usize bufferLength) noexcept
{
    VXIO_ASSERT_NOTNULL(buffer);
    VXIO_ASSERT_NE(bufferLength, 0);

    if (not initialized) {
        return init();
    }

    state.readVoxels = 0;

    if (state.resumeCount != 0) {
        if (not resumeWritingToBuffer(buffer, bufferLength) || state.readVoxels == bufferLength) {
            return ReadResult::incomplete(state.readVoxels);
        }
        VXIO_DEBUG_ASSERT_EQ(state.resumeCount, 0);
    }

    while (true) {
        auto result = readNextVoxels(buffer, bufferLength);
        if (result.type != ResultCode::READ_OBJECT_END) return result;
    }
}

bool Reader::resumeWritingToBuffer(Voxel64 buffer[], usize bufferLength)
{
    auto lim = std::min<u32>(state.resumeCount, static_cast<u32>(bufferLength));
    u8 i = 0;
    for (; i != lim; ++i) {
        buffer[state.readVoxels++] = {posOf(state.index++).cast<i64>(), {this->color}};
    }
    state.resumeCount -= lim;

    return state.resumeCount != 0;
}

ReadResult Reader::readNextVoxels(Voxel64 buffer[], usize bufferLength)
{
    static_assert(sizeof(voxelBuffer) == 2);

    if (state.index == header.volume) {
        return ReadResult::end(state.readVoxels);
    }

    stream.read(voxelBuffer, sizeof(voxelBuffer));
    VXIO_NO_EOF();

    u8 value = voxelBuffer[0];
    u8 count = voxelBuffer[1];

    if (state.index + count > header.volume) {
        return ReadResult::parseError(state.lineNum, "voxel range extends beyond end of file");
    }

    switch (value) {
    case 0: {
        state.index += count;
        return ReadResult::nextObject(state.readVoxels);
    }

    case 1: {
        const auto lim = std::min<u32>(count, static_cast<u32>(bufferLength));
        u8 i = 0;
        for (; i != lim; ++i) {
            buffer[state.readVoxels++] = {posOf(state.index++).cast<i64>(), {this->color}};
        }
        if (lim < count) {
            state.resumeCount = count - lim;
            return ReadResult::incomplete(state.readVoxels);
        }
        return ReadResult::nextObject(state.readVoxels);
    }

    default:
        return ReadResult::unexpectedSymbol(state.lineNum,
                                            "voxel value must be 0 or 1 (is " + std::to_string(value) + ")");
    }
}

Vec3u64 Reader::posOf(u64 index)
{
    // y id the fastest-growing axis, followed by z, followed by x:
    //     index = x * width * height + z * width + y;
    const Vec3u32 &dim = header.dim;

    u64 x = index / dim.y() / dim.z();
    u64 y = index % dim.y();
    u64 z = index / dim.y() % dim.z();
    return Vec3u64{x, y, z};
}

#undef VXIO_PARSE_SAFELY

}  // namespace voxelio::binvox
