#include "voxelio/image.hpp"

#include "voxelio/intdiv.hpp"

namespace voxelio {

namespace {

// ENCODERS ============================================================================================================

constexpr void encodeV1(Color32 rgb, u8 *out, usize bitOffset)
{
    u8 mask = static_cast<u8>(1 << (7 - bitOffset));
    bool bit = rgb.a != 0;
    out[0] = (out[0] & ~mask) | static_cast<u8>((bit << (7 - bitOffset)));
}

constexpr void encodeV8(Color32 rgb, u8 *out, usize)
{
    out[0] = static_cast<u8>(rgb.a);
}

constexpr void encodeVA16(Color32 rgb, u8 *out, usize)
{
    out[0] = static_cast<u8>(rgb.r);
    out[1] = static_cast<u8>(rgb.a);
}

constexpr void encodeRgb24(Color32 rgb, u8 *out, usize)
{
    out[0] = static_cast<u8>(rgb.r);
    out[1] = static_cast<u8>(rgb.g);
    out[2] = static_cast<u8>(rgb.b);
}

constexpr void encodeRgba32(Color32 rgb, u8 *out, usize)
{
    out[0] = static_cast<u8>(rgb.r);
    out[1] = static_cast<u8>(rgb.g);
    out[2] = static_cast<u8>(rgb.b);
    out[3] = static_cast<u8>(rgb.a);
}

constexpr void encodeArgb32(Color32 rgb, u8 *out, usize)
{
    out[0] = static_cast<u8>(rgb.a);
    out[1] = static_cast<u8>(rgb.r);
    out[2] = static_cast<u8>(rgb.g);
    out[3] = static_cast<u8>(rgb.b);
}

constexpr detail::RgbEncoder encoderOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return encodeV1;
    case ColorFormat::V8: return encodeV8;
    case ColorFormat::VA16: return encodeVA16;
    case ColorFormat::RGB24: return encodeRgb24;
    case ColorFormat::RGBA32: return encodeRgba32;
    case ColorFormat::ARGB32: return encodeArgb32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

// DECODERS ============================================================================================================

constexpr Color32 decodeV1(const u8 *in, usize bitOffset)
{
    bool bit = (*in >> (7 - bitOffset)) & 1;
    // we fill the channel with 1s by underflowing to 0xff if the bit is set
    u8 channel = (not bit) - 1;
    return {channel, channel, channel};
}

constexpr Color32 decodeV8(const u8 *in, usize)
{
    return {*in, *in, *in};
}

constexpr Color32 decodeVA16(const u8 *in, usize)
{
    return {in[0], in[0], in[0], in[1]};
}

constexpr Color32 decodeRgb24(const u8 *in, usize)
{
    return {in[0], in[1], in[2]};
}

constexpr Color32 decodeRgba32(const u8 *in, usize)
{
    return {in[0], in[1], in[2], in[3]};
}

constexpr Color32 decodeArgb32(const u8 *in, usize)
{
    return {in[3], in[0], in[1], in[2]};
}

constexpr detail::RgbDecoder decoderOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return decodeV1;
    case ColorFormat::V8: return decodeV8;
    case ColorFormat::VA16: return decodeVA16;
    case ColorFormat::RGB24: return decodeRgb24;
    case ColorFormat::RGBA32: return decodeRgba32;
    case ColorFormat::ARGB32: return decodeArgb32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

}  // namespace

// FACTORY AND IMAGE ===================================================================================================

usize Image::contentSizeOf(usize w, usize h, ColorFormat format)
{
    return divCeil(w * h * bitSizeOf(format), 8u);
}

Image::Image(usize w, usize h, ColorFormat format, std::unique_ptr<u8[]> content, WrapMode wrapMode)
    : contentSize{contentSizeOf(w, h, format)}
    , content{std::move(content)}
    , w{w}
    , h{h}
    , bitsPerPixel{bitSizeOf(format)}
    , f{format}
    , encoder{encoderOf(format)}
    , decoder{decoderOf(format)}
{
    setWrapMode(wrapMode);
}

Image::Image(usize w, usize h, ColorFormat format, WrapMode wrapMode)
    : Image(w, h, format, std::make_unique<u8[]>(contentSizeOf(w, h, format)))
{
    setWrapMode(wrapMode);
}

}  // namespace voxelio
