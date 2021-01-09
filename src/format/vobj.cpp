#include "voxelio/format/vobj.hpp"

#include "voxelio/color.hpp"
#include "voxelio/macro.hpp"
#include "voxelio/stringify.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_set>

namespace voxelio::vobj {

VobjWriteHelper::OffsetGuard::OffsetGuard(VobjWriteHelper *parent, Vec3i64 offset)
    : parent{parent}, offset{std::move(offset)}
{
    parent->offset += this->offset;
}

VobjWriteHelper::OffsetGuard::~OffsetGuard()
{
    parent->offset -= this->offset;
}

size_t VobjWriteHelper::voxelsWritten()
{
    return index;
}

void VobjWriteHelper::setBaseOffset(Vec3i64 offset)
{
    this->offset = std::move(offset);
}

[[nodiscard]] VobjWriteHelper::OffsetGuard VobjWriteHelper::addGuardedOffset(Vec3i64 offset)
{
    return OffsetGuard{this, std::move(offset)};
}

void VobjWriteHelper::setColorFormat(ColorFormat format)
{
    this->colorFormat = format;
}

void VobjWriteHelper::resetBuffer(Voxel64 buffer[], size_t bufferSize)
{
    VXIO_DEBUG_ASSERT_CONSEQUENCE(bufferSize != 0, buffer != nullptr);
    this->buffer = buffer;
    this->bufferSize = bufferSize;
    this->index = 0;
}

bool VobjWriteHelper::canWrite()
{
    return index != bufferSize;
}

bool VobjWriteHelper::isFull()
{
    return index == bufferSize;
}

void VobjWriteHelper::write(Vec3i64 pos, argb32 color)
{
    VXIO_DEBUG_ASSERT_LT(index, bufferSize);
    buffer[index++] = Voxel64{pos + offset, {color}};
}

ArrayU8 makeArrayU8(size_t size)
{
    return std::make_unique<u8[]>(size);
}

ArrayU8 copyArrayU8(const ArrayU8 &array, size_t bytes)
{
    ArrayU8 result = makeArrayU8(bytes);
    std::memcpy(result.get(), array.get(), bytes);
    return result;
}

// see http://www.cs.nott.ac.uk/~psarb2/G51MPC/slides/NumberLogic.pdf
u32 divCeil(u32 numerator, u32 denominator)
{
    // u64 to ensure overflow safety
    u64 newNumerator = u64(numerator) + u64(denominator) - 1;
    return static_cast<u32>(newNumerator / denominator);
}

template <typename UInt>
constexpr u64 zeroToMaxPlusOne(UInt num)
{
    return (num == 0) ? static_cast<u64>(std::numeric_limits<decltype(num)>::max()) + 1 : num;
};

static const std::unordered_set<std::string> recognizedExtensions{
    EXT_DEBUG, EXT_EXISTENCE_ARRAY, EXT_GROUPS, EXT_16_BIT_ARRAY, EXT_32_BIT_ARRAY};

static const std::unordered_set<u8> recognizedColorFormats{RGB24, ARGB32, V8, AV16};

static const std::unordered_set<u8> recognizedDataFormats{EMPTY, LIST, ARRAY_POSITIONED, ARRAY_TILED};

static const std::unordered_set<u8> recognizedPaletteBits{0, 8, 16, 32};

// IReader overrides

[[nodiscard]] ReadResult Reader::init() noexcept
{
    VXIO_FORWARD_ERROR(readHeader());
    return readContent(false);
}

[[nodiscard]] ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    VXIO_ASSERT_NOTNULL(buffer);
    VXIO_ASSERT_NE(bufferLength, 0);

    if (not initialized) {
        auto result = init();
        if (result.isBad() || result.isEnd()) return result;
        VXIO_DEBUG_ASSERT(initialized);
        return result;
    }

    writeHelper.resetBuffer(buffer, bufferLength);
    return readContent(true);
}

// Implementation

bool Reader::pushGroup(GroupHeader group)
{
    if (group.name.empty()) {
        groupStack.emplace(std::move(group));
    }

    auto &otherNames = groupStack.top().childNames;
    if (otherNames.find(group.name) != otherNames.end()) {
        return false;
    }

    otherNames.emplace(group.name);
    groupStack.emplace(std::move(group));
    return true;
}

bool Reader::popGroups(size_t count)
{
    if (count >= groupStack.size()) {
        return false;
    }
    for (size_t i = 0; i < count; ++i) {
        groupStack.pop();
    }
    return true;
}

argb32 Reader::decodeColor(u8 data[])
{
    switch (colorFormat) {
    case RGB24: return Color32{data[0], data[1], data[2], u8{0xFF}};
    case ARGB32: return Color32{data[1], data[2], data[3], data[0]};
    case V8: return Color32{data[0], data[0], data[0], u8{0xFF}};
    case AV16: return Color32{data[1], data[1], data[1], data[0]};
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
    return 0;
}

[[nodiscard]] ReadResult Reader::readHeader()
{
    std::string magic;
    VXIO_FORWARD_ERROR(readString(12, magic));
    if (magic != "model/x-vobj") {
        return ReadResult::unexpectedMagic(0, magic);
    }

    VXIO_FORWARD_ERROR(skipString());  // supportUrl
    VXIO_FORWARD_ERROR(readExtensions());
    VXIO_FORWARD_ERROR(readColorFormat());
    VXIO_FORWARD_ERROR(readPalette());

    u32 metaSize = stream.readBig<u32>();
    VXIO_NO_EOF();
    if (metaSize != 0) {
        VXIO_FORWARD_ERROR(skipString());  // vendorName
        stream.seekRelative(metaSize);
    }
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readArrayU8(size_t bytes, ArrayU8 &out)
{
    static_assert(std::is_same_v<unsigned char, u8>, "reinterpretation of char as u8 impossible");
    out = makeArrayU8(bytes);
    stream.read(out.get(), bytes);
    VXIO_NO_EOF();
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readString(size_t length, std::string &out)
{
    out.resize(length);
    stream.read(reinterpret_cast<u8 *>(out.data()), length);
    VXIO_NO_EOF();
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readString(std::string &out)
{
    u16 length = stream.readBig<u16>();
    VXIO_NO_EOF();
    return readString(length, out);
}

[[nodiscard]] ReadResult Reader::skipString()
{
    u16 length = stream.readBig<u16>();
    VXIO_NO_EOF();
    stream.seekRelative(length);
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readExtensions()
{
    u16 extensionsSize = stream.readBig<u16>();
    VXIO_NO_EOF();
    std::unordered_set<std::string> extSet;

    for (size_t i = 0; i < extensionsSize; ++i) {
        std::string extension;
        VXIO_FORWARD_ERROR(readString(extension));
        if (bool unrecognized = recognizedExtensions.find(extension) != recognizedExtensions.end(); unrecognized) {
            return ReadResult::unknownFeature(stream.position(), extension);
        }
        extSet.insert(std::move(extension));
    }

    ext.debug = extSet.find(EXT_DEBUG) != extSet.end();
    ext.exArr = extSet.find(EXT_EXISTENCE_ARRAY) != extSet.end();
    ext.group = extSet.find(EXT_GROUPS) != extSet.end();
    ext.arr16 = extSet.find(EXT_16_BIT_ARRAY) != extSet.end();
    ext.arr32 = extSet.find(EXT_32_BIT_ARRAY) != extSet.end();

    if (ext.arr16 && ext.arr32) {
        return ReadResult::parseError(stream.position(), "extension conflict between arr16 and arr32");
    }

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readPalette()
{
    palette.bits = stream.readU8();
    VXIO_NO_EOF();
    if (recognizedPaletteBits.find(palette.bits) == recognizedPaletteBits.end()) {
        return ReadResult::unexpectedSymbol(stream.position(), "unrecognized palette bits: " + stringify(palette.bits));
    }

    switch (palette.bits) {
    case 0: {
        palette.size = 0;
        return ReadResult::ok();
    }
    case 8: {
        palette.size = zeroToMaxPlusOne(stream.readU8());
        break;
    }
    case 16: {
        palette.size = zeroToMaxPlusOne(stream.readBig<u16>());
        break;
    }
    case 32: {
        palette.size = zeroToMaxPlusOne(stream.readBig<u32>());
        break;
    }
    default: {
        VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
    }
    VXIO_NO_EOF();
    VXIO_FORWARD_ERROR(readArrayU8(palette.size * colorByteCount, palette.content));
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readColorFormat()
{
    u8 result = stream.readU8();
    VXIO_NO_EOF();
    if (recognizedColorFormats.find(result) == recognizedColorFormats.end()) {
        return ReadResult::unexpectedSymbol(stream.position(), "unknown color format: 0x" + stringifyHex(result));
    }
    colorFormat = static_cast<ColorFormat>(result);
    colorByteCount = (static_cast<u8>(colorFormat) & u8{0x3f}) / u8{8};
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readDataFormat()
{
    u8 result = stream.readU8();
    VXIO_NO_EOF();
    if (recognizedDataFormats.find(result) == recognizedDataFormats.end()) {
        return ReadResult::unexpectedSymbol(stream.position(), "unknown data format: 0x" + stringifyHex(result));
    }
    state.format = static_cast<DataFormat>(result);
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readDimensions()
{
    constexpr auto transform = [](auto num) -> u64 {
        return (num == 0) ? static_cast<u64>(std::numeric_limits<decltype(num)>::max()) + 1 : num;
    };
    static_assert(transform(u8{00}) == 0x100);
    static_assert(transform(u16{0}) == 0x10000);
    static_assert(transform(u32{0}) == 0x100000000);

    if (ext.arr32) {
        state.arrDims[0] = transform(stream.readBig<u32>());
        state.arrDims[1] = transform(stream.readBig<u32>());
        state.arrDims[2] = transform(stream.readBig<u32>());
        VXIO_NO_EOF();
        // TODO overflow safety
        state.arrLim = state.arrDims[0] * state.arrDims[1] * state.arrDims[2];
    }
    else {
        if (ext.arr16) {
            state.arrDims[0] = transform(stream.readBig<u16>());
            state.arrDims[1] = transform(stream.readBig<u16>());
            state.arrDims[2] = transform(stream.readBig<u16>());
        }
        else {
            state.arrDims[0] = transform(stream.readU8());
            state.arrDims[1] = transform(stream.readU8());
            state.arrDims[2] = transform(stream.readU8());
        }
        VXIO_NO_EOF();
        state.arrLim = state.arrDims[0] * state.arrDims[1] * state.arrDims[2];
    }

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readVoxel(argb32 &out)
{
    if (palette.bits == 0) {
        ArrayU8 result;
        VXIO_FORWARD_ERROR(readArrayU8(colorByteCount, result));
        out = decodeColor(result.get());
        return ReadResult::ok();
    }
    else if (palette.size == 1) {
        out = decodeColor(palette.content.get());
        return ReadResult::ok();
    }
    else {
        return readVoxelUsingPalette(out);
    }
}

[[nodiscard]] ReadResult Reader::readVoxelUsingPalette(argb32 &out)
{
    u32 index;
    switch (palette.bits) {
    case 8: {
        index = stream.readU8();
        break;
    }
    case 16: {
        index = stream.readBig<u16>();
        break;
    }
    case 32: {
        index = stream.readBig<u32>();
        break;
    }
    default: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
    VXIO_NO_EOF();
    out = decodeColor(palette.content.get() + index * colorByteCount);
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readContent(bool resume)
{
    if (ext.group) {
        if (not resume) {
            state.grpIndex = 0;
            state.grpLim = stream.readBig<u32>();
            VXIO_NO_EOF();
        }

        return state.grpIndex++ < state.grpLim ? readGroup(resume) : ReadResult::end(writeHelper.voxelsWritten());
    }
    else {
        if (not resume) {
            VXIO_FORWARD_ERROR(readDataFormat());
        }
        auto result = readTypedData(resume);
        return (result.isBad() || result.type != ResultCode::READ_OBJECT_END) ? result
                                                                              : ReadResult::end(result.voxelsRead);
    }
}

[[nodiscard]] ReadResult Reader::readGroup(bool resume)
{
    if (not resume) {
        u16 popCount = stream.readBig<u16>();
        VXIO_NO_EOF();
        if (not popGroups(popCount)) {
            return ReadResult::parseError(stream.position(), "too many groups popped (" + stringify(popCount) + ")");
        }

        std::string groupName;
        VXIO_FORWARD_ERROR(readString(groupName));
        Vec3i32 pos;
        stream.readBig<3>(pos.data());

        if (not pushGroup({groupName, pos})) {
            return ReadResult::parseError(stream.position(), "duplicate group \"" + groupName + "\"");
        }

        VXIO_FORWARD_ERROR(readDataFormat());
    }

    return readTypedData(resume);
}

[[nodiscard]] ReadResult Reader::readTypedData(bool resume)
{
    ReadResult result = ReadResult::nextObject();

    if (state.format == EMPTY) {
        return result;
    }

    if (not resume) {
        if (state.format == ARRAY_TILED) {
            VXIO_FORWARD_ERROR(readDimensions());
        }

        state.datIndex = 0;
        state.datLim = stream.readBig<u32>();
        VXIO_NO_EOF();
    }

    // we will always return OK (incomplete read) if there are more arrays or voxels
    // if all were read NEXT_OBJECT will be returned
#define DO_READ(function)                                                        \
    for (; state.datIndex < state.datLim; ++state.datIndex) {                    \
        result = function(resume);                                               \
        if (result.isBad() || result.type != ResultCode::READ_OBJECT_END) break; \
        resume = false;                                                          \
    }                                                                            \
    break;

    // we handle this here instead of in readPositionedVoxel because it would be too wasteful to make a last-minute
    // decision for every voxel
    if (not initialized && state.format == LIST) {
        initialized = true;
        return ReadResult::ok();
    }

    switch (state.format) {
    case LIST: DO_READ(readPositionedVoxel);
    case ARRAY_POSITIONED: DO_READ(readPositionedArray);
    case ARRAY_TILED: DO_READ(readTiledArray);
    case EMPTY: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }

    return result;

#undef DO_READ
}

[[nodiscard]] ReadResult Reader::readPositionedVoxel(bool)
{
    VXIO_DEBUG_ASSERT(initialized);
    if (writeHelper.isFull()) {
        return ReadResult::ok(writeHelper.voxelsWritten());
    }

    Vec3i32 pos;
    stream.readBig<3>(pos.data());
    VXIO_NO_EOF();
    argb32 color;
    VXIO_FORWARD_ERROR(readVoxel(color));
    writeHelper.write(pos.cast<i64>(), color);
    return ReadResult::nextObject(writeHelper.voxelsWritten());
}

[[nodiscard]] ReadResult Reader::readPositionedArray(bool resume)
{
    if (not resume) {
        stream.readBig<3, i64>(state.arrPos.data());
        VXIO_NO_EOF();
        VXIO_FORWARD_ERROR(readDimensions());
        state.arrIndex = 0;
    }

    return readArrayContent(resume);
}

[[nodiscard]] ReadResult Reader::readTiledArray(bool resume)
{
    if (not resume) {
        stream.readBig<3, i64>(state.arrPos.data());
        VXIO_NO_EOF();
        state.arrPos = mul(state.arrPos, state.arrDims.cast<i64>());
    }

    return readArrayContent(resume);
}

[[nodiscard]] ReadResult Reader::readArrayContent(bool resume)
{
    const auto offsetGuard = writeHelper.addGuardedOffset(state.arrPos);

    return ext.exArr ? readArrayWithContentExistence(resume) : readArrayContentWithoutExistence(resume);
}

[[nodiscard]] ReadResult Reader::readArrayWithContentExistence(bool resume)
{
    if (not resume) {
        u32 existenceBytes = divCeil(static_cast<u32>(state.arrLim), 8);
        VXIO_FORWARD_ERROR(readArrayU8(existenceBytes, state.existArr));
        state.arrLim = stream.readBig<u32>();
        VXIO_NO_EOF();
        state.arrIndex = 0;
        if (not initialized) {
            initialized = true;
            return ReadResult::ok();
        }
    }
    VXIO_DEBUG_ASSERT(initialized);

    const u64 limX = state.arrDims.x();
    const u64 limY = state.arrDims.y();
    const u64 limZ = state.arrDims.z();

    for (; state.arrZ < limZ; ++state.arrZ) {
        for (; state.arrY < limY; ++state.arrY) {
            for (; state.arrX < limX; ++state.arrX, ++state.arrIndex) {
                if (writeHelper.isFull()) {
                    return ReadResult::ok(writeHelper.voxelsWritten());
                }

                size_t superIndex = state.arrIndex / 8;
                u8 maskBit = u8{0b10000000} >> (state.arrIndex % 8);
                if ((state.existArr[superIndex] & maskBit) == 0) continue;

                argb32 color;
                VXIO_FORWARD_ERROR(readVoxel(color));
                writeHelper.write({state.arrX, state.arrY, state.arrZ}, color);
            }
            state.arrX = 0;
        }
        state.arrY = 0;
    }

    return ReadResult::nextObject(writeHelper.voxelsWritten());
}

[[nodiscard]] ReadResult Reader::readArrayContentWithoutExistence(bool)
{
    if (not initialized) {
        initialized = true;
        return ReadResult::ok();
    }
    VXIO_DEBUG_ASSERT(initialized);

    const u64 limX = state.arrDims.x();
    const u64 limY = state.arrDims.y();
    const u64 limZ = state.arrDims.z();

    for (; state.arrZ < limZ; ++state.arrZ) {
        for (; state.arrY < limY; ++state.arrY) {
            for (; state.arrX < limX; ++state.arrX) {
                if (writeHelper.isFull()) {
                    return ReadResult::ok(writeHelper.voxelsWritten());
                }

                argb32 color;
                VXIO_FORWARD_ERROR(readVoxel(color));
                writeHelper.write({state.arrX, state.arrY, state.arrZ}, color);
            }
            state.arrX = 0;
        }
        state.arrY = 0;
    }

    return ReadResult::nextObject(writeHelper.voxelsWritten());
}

}  // namespace voxelio::vobj
