#include "voxelio/format/qef.hpp"

#include "voxelio/filetype.hpp"
#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"
#include "voxelio/parse.hpp"
#include "voxelio/stringify.hpp"
#include "voxelio/stringmanip.hpp"

#include <cmath>
#include <stdexcept>

namespace voxelio::qef {

namespace {

constexpr const char *CSTR_MAGIC = magicOf(FileType::QUBICLE_EXCHANGE);
constexpr const char *CSTR_VERSION = "Version 0.2";
constexpr const char *CSTR_SUPPORT_URL = "www.minddesk.com";

const std::string PREAMBLE = CSTR_MAGIC + std::string{'\n'} + CSTR_VERSION + '\n' + CSTR_SUPPORT_URL + '\n';

constexpr size_t HEADER_LINE_COUNT = 5;

Color32 roundColor(const float rgb[3])
{
    u8 result[3];
    for (size_t i = 0; i < 3; ++i) {
        result[i] = static_cast<u8>(std::lround(rgb[i] * 255));
    }
    return {result[0], result[1], result[2]};
}

template <typename T>
std::vector<T> parseMultiple(const std::string &line) noexcept(false)
{
    std::vector<T> result;
    std::vector<std::string> splits = splitAtDelimiter(line, ' ');
    result.resize(splits.size());

    for (size_t i = 0; i < splits.size(); i++) {
        if (not parse(splits[i], result[i])) {
            throw std::runtime_error("failed to parse \"" + line + "\"");
        }
    }
    return result;
}

std::string vecToStringLine(Vec3u32 v)
{
    return stringify(v[0]) + ' ' + stringify(v[1]) + ' ' + stringify(v[2]) + '\n';
}

}  // namespace

ReadResult Reader::init() noexcept
{
    if (initialized) {
        return {0, ResultCode::WARNING_DOUBLE_INIT};
    }
    initialized = true;

    if (not stream.good()) {
        return ReadResult::ioError(0, "stream is already not good() before init()");
    }

    std::string line;

    VXIO_FORWARD_ERROR(readLine(line));  // 1
    if (line != CSTR_MAGIC) {
        return ReadResult::unexpectedMagic(1, line);
    }

    VXIO_FORWARD_ERROR(readLine(line));  // 2
    if (line == CSTR_VERSION) {
        VXIO_LOG(SPAM, "parsing file of version '" + line + "'");
    }
    else {
        return ReadResult::unknownVersion(2, "Expected \"Version 0.2\", got \"" + line + "\"");
    }

    // skip support url (www.minddesk.com)
    VXIO_FORWARD_ERROR(readLine(line));  // 3

    VXIO_FORWARD_ERROR(readLine(line));  // 4
    VXIO_FORWARD_ERROR(parseDimensions(line));
    VXIO_LOG(SPAM, "qef dimensions: " + this->dimensions.toString());

    VXIO_LOG(SPAM,
             "parsing qef of dimensions " + stringify(dimensions[0]) + "x" + stringify(dimensions[1]) + "x" +
                 stringify(dimensions[2]));

    VXIO_FORWARD_ERROR(readLine(line));  // 5
    VXIO_FORWARD_ERROR(parseColorCount(line));
    VXIO_LOG(SPAM, "parsing " + stringify(paletteSize) + " colors ...");

    if (paletteSize == 0) {
        VXIO_LOG(SPAM, "zero colors, returning end result");
        return ReadResult::end();
    }

    for (size_t i = 0; i < paletteSize; ++i) {
        VXIO_FORWARD_ERROR(readLine(line));
        VXIO_FORWARD_ERROR(parseColorDefinition(i + 6, line));
    }

    return ReadResult::nextObject();
}

ReadResult Reader::read(Voxel32 buffer[], size_t bufferLength)
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::qef::Reader::init() in read()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::qef::Reader::init() in read()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

ReadResult Reader::doRead() noexcept
{
    if (stream.eof()) {
        return ReadResult::end(writeHelper.voxelsWritten());
    }

    auto lineNum = paletteSize + 6;
    std::string line;
    while (writeHelper.canWrite()) {
        VXIO_FORWARD_ERROR(readLine(line));
        if (not line.empty()) {
            VXIO_FORWARD_ERROR(parseVoxelDefinition(lineNum, line));
        }
        ++lineNum;
        if (stream.eof()) {
            return ReadResult::end(writeHelper.voxelsWritten());
        }
    }

    return ReadResult::ok(writeHelper.voxelsWritten());
}

[[nodiscard]] ReadResult Reader::readLine(std::string &out) noexcept
{
    if (stream.eof()) {
        return ReadResult::unexpectedEof(0, "already reached eof before reading a line");
    }
    stream.readLineTo(out);
    if (stream.err()) {
        return ReadResult::ioError(0, "IO error when reading line");
    }
    return ReadResult::ok();
}

ReadResult Reader::parseDimensions(const std::string &line) noexcept
{
    auto dims = parseMultiple<size_t>(line);
    if (dims.size() < 3) return ReadResult::parseError(4, "fewer than 3 dimensions");
    dimensions = {dims[0], dims[1], dims[2]};
    return ReadResult::ok();
}

ReadResult Reader::parseColorCount(const std::string &line) noexcept
{
    if (not parse(line, this->paletteSize)) return ReadResult::parseError(5, "failed to parse \"" + line + "\"");
    this->palette = std::make_unique<argb32[]>(this->paletteSize);
    return ReadResult::ok();
}

ReadResult Reader::parseColorDefinition(u64 num, const std::string &line) noexcept
{
    auto rgb = parseMultiple<float>(line);
    if (rgb.size() < 3) {
        return ReadResult::parseError(num, "color has fewer than 3 channels");
    }
    palette[paletteIndex++] = roundColor(rgb.data());
    return ReadResult::ok();
}

ReadResult Reader::parseVoxelDefinition(u64 num, const std::string &line) noexcept
{
    VXIO_DEBUG_ASSERT(initialized);
    std::vector<u32> ints = parseMultiple<u32>(line);
    if (ints.size() < 4) return ReadResult::parseError(num, "voxel has fewer than 4 values");
    Vec3i64 pos = {ints[0], ints[1], ints[2]};
    auto readPaletteIndex = ints[3];
    if (readPaletteIndex >= paletteSize) {
        return ReadResult::parseError(num,
                                      "palette index " + stringify(readPaletteIndex) +
                                          " is out of range (paletteSize = " + stringify(paletteSize) + ")");
    }

    writeHelper.emplace(pos, palette[readPaletteIndex]);
    return ReadResult::ok();
}

//======================================================================================================================

ResultCode Writer::init() noexcept
{
    if (initialized) {
        return ResultCode::WARNING_DOUBLE_INIT;
    }
    initialized = true;

    if (canvasDims == std::nullopt) {
        this->err = {0, "canvas dimensions must be set"};
        return ResultCode::USER_ERROR_MISSING_BOUNDARIES;
    }

    VXIO_FORWARD_ERROR(writeString(PREAMBLE));
    VXIO_FORWARD_ERROR(writeString(vecToStringLine(*canvasDims)));
    VXIO_FORWARD_ERROR(writePalette());

    return ResultCode::OK;
}

ResultCode Writer::writePalette() noexcept
{
    auto paletteSize = palette().size();
    auto paletteContent = palette().build();

    VXIO_LOG(SPAM, "writing palette with " + stringify(paletteSize) + " entries");
    VXIO_FORWARD_ERROR(writeString(stringify(paletteSize) + '\n'));

    for (size_t i = 0; i < paletteSize; ++i) {
        VXIO_FORWARD_ERROR(writeColorLine(paletteContent[i]));
    }

    return ResultCode::OK;
}

ResultCode Writer::write(Voxel32 buffer[], usize bufferLength) noexcept
{
    if (not initialized) {
        VXIO_FORWARD_ERROR(init());
    }
    VXIO_DEBUG_ASSERT(initialized);

    if (bufferLength != 0 && palette().empty()) {
        this->err = {0, "can't write qef without a palette (palette is empty)"};
        return ResultCode::USER_ERROR_MISSING_PALETTE;
    }

    for (size_t i = 0; i < bufferLength; ++i) {
        VXIO_FORWARD_ERROR(writeVoxelLine(buffer[i]));
    }
    return ResultCode::OK;
}

ResultCode Writer::writeString(std::string line) noexcept
{
    stream.writeString(line);
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

ResultCode Writer::writeColorLine(Color32 color) noexcept
{
    // Using proper rounding, a precision of 3 would already be enough.
    // We use a precision of 4 in case other software uses improper rounding, such as floor.
    constexpr auto stringifyChannel = [](u8 channel) -> std::string {
        constexpr unsigned precision = 4;
        return stringifyFractionRpad(channel, 255u, precision);
    };

    stream.writeString(stringifyChannel(color.r) + ' ');
    stream.writeString(stringifyChannel(color.g) + ' ');
    stream.writeString(stringifyChannel(color.b) + '\n');
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

ResultCode Writer::writeVoxelLine(Voxel32 v) noexcept
{
    VXIO_FORWARD_ERROR(verifyVoxel(v));

    stream.writeString(stringify(v.pos[0]) + ' ' + stringify(v.pos[1]) + ' ' + stringify(v.pos[2]) + ' ' +
                       stringify(v.index) + '\n');
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

ResultCode Writer::verifyVoxel(Voxel32 voxel) noexcept
{
    const auto &canvasDimsValue = *canvasDims;
    const auto &pos = voxel.pos;

    for (size_t i = 0; i < 3; ++i) {
        if (pos[i] < 0 || static_cast<u32>(pos[i]) > canvasDimsValue[i]) {
            // TODO use line number
            this->err = {0,
                         "the given voxel " + pos.toString() +
                             " is outside the given canvas boundaries: " + canvasDimsValue.toString()};
            return ResultCode::WRITE_ERROR_POSITION_OUT_OF_BOUNDS;
        }
    }
    if (voxel.index >= palette().size()) {
        this->err = {0,
                     "the voxel's palette index " + stringify(voxel.index) +
                         " is outside the palette (size = " + stringify(palette().size()) + ")"};
        return ResultCode::WRITE_ERROR_INDEX_OUT_OF_BOUNDS;
    }

    return ResultCode::OK;
}

//======================================================================================================================

VoxelArray Deserializer::read() noexcept(false)
{
    size_t lineNum = 0;

    std::string line;
    while (true) {
        streamWrapper.readLineTo(line);
        if (streamWrapper.eof()) {
            if (not line.empty()) {
                // this is only necessary for files which were not properly terminated with a \n
                // some software might export such files and we don't want to lose the last line
                parseLine(++lineNum, line);
            }
            break;
        }
        parseLine(++lineNum, line);
    }

    if (lineNum < HEADER_LINE_COUNT) {
        throw std::runtime_error("less than 5 lines read, QEF incomplete");
    }

    VXIO_LOG(
        DEBUG,
        "completed parsing qef (" + stringify(voxels->countVoxels()) + "/" + stringify(voxels->volume()) + " voxels)")

    return std::move(voxels.value());
}

void Deserializer::parseLine(size_t num, const std::string &line) noexcept(false)
{
    constexpr size_t magicLine = 1;
    constexpr size_t versionLine = 2;
    constexpr size_t urlLine = 3;
    constexpr size_t dimensionsLine = 4;
    constexpr size_t colorCountLine = 5;
    constexpr size_t firstColorLine = 6;

    if (num == magicLine || num == urlLine) return;  // header lines

    if (num == versionLine) {
        if (line == "Version 0.2") {
            VXIO_LOG(DEBUG, "parsing file of version '" + line + "'");
        }
        else {
            throw std::runtime_error("version '" + line + "' not supported");
        }
    }

    else if (num == dimensionsLine) {
        parseDimensions(line);
        VXIO_LOG(DEBUG, "QEF has dimensions " + voxels->dimensions().toString());
    }

    else if (num == colorCountLine) {
        colorCount = parseColorCount(line);
        VXIO_LOG(DEBUG, "parsing " + stringify(colorCount) + " colors ...");
    }

    else if (size_t firstVoxelLine = firstColorLine + colorCount; num < firstVoxelLine) {
        parseColorDefinition(num, line);
    }

    else {
        parseVoxelDefinition(num, line);
    }
}

void Deserializer::parseDimensions(const std::string &line) noexcept(false)
{
    auto dims = parseMultiple<size_t>(line);
    if (dims.size() < 3) {
        throw std::runtime_error("fewer than 3 dimensions");
    }
    voxels.emplace(dims[0], dims[1], dims[2]);
}

size_t Deserializer::parseColorCount(const std::string &line) noexcept(false)
{
    size_t result;
    if (not parse(line, result)) {
        throw std::runtime_error("failed to parse \"" + line + "\"");
    }
    return result;
}

void Deserializer::parseColorDefinition(size_t num, const std::string &line) noexcept(false)
{
    auto rgb = parseMultiple<float>(line);
    if (rgb.size() < 3) {
        throw std::runtime_error(stringify(num) + ": color has fewer than 3 channels");
    }

    colors.push_back(roundColor(rgb.data()));
}

void Deserializer::parseVoxelDefinition(size_t num, const std::string &line) noexcept(false)
{
    VXIO_DEBUG_ASSERT(voxels.has_value());
    auto ints = parseMultiple<u32>(line);
    if (ints.size() < 4) {
        throw std::runtime_error(stringify(num) + ": voxel has fewer than 4 values");
    }
    Vec3size pos = {ints[0], ints[1], ints[2]};
    if (voxels->contains(pos)) {
        VXIO_LOG(WARNING, "Duplicate QEF voxel at " + pos.toString());
    }
    (*voxels)[pos] = colors[ints[3]];
}

}  // namespace voxelio::qef
