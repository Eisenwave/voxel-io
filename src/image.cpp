#include "image.hpp"

#include "intdiv.hpp"

namespace voxelio {

static constexpr void encodeV1(Color32 rgb, u8 *out, size_t bitOffset)
{
    u8 mask = static_cast<u8>(1 << (7 - bitOffset));
    bool bit = rgb.a != 0;
    out[0] = (out[0] & ~mask) | static_cast<u8>((bit << (7 - bitOffset)));
}

static constexpr void encodeV8(Color32 rgb, u8 *out, size_t)
{
    out[0] = static_cast<u8>(rgb.a);
}

static constexpr void encodeRgb24(Color32 rgb, u8 *out, size_t)
{
    out[0] = static_cast<u8>(rgb.r);
    out[1] = static_cast<u8>(rgb.g);
    out[2] = static_cast<u8>(rgb.b);
}

static constexpr void encodeRgba32(Color32 rgb, u8 *out, size_t)
{
    out[0] = static_cast<u8>(rgb.r);
    out[1] = static_cast<u8>(rgb.g);
    out[2] = static_cast<u8>(rgb.b);
    out[3] = static_cast<u8>(rgb.a);
}

static constexpr void encodeArgb32(Color32 rgb, u8 *out, size_t)
{
    out[0] = static_cast<u8>(rgb.a);
    out[1] = static_cast<u8>(rgb.r);
    out[2] = static_cast<u8>(rgb.g);
    out[3] = static_cast<u8>(rgb.b);
}

static constexpr detail::RgbEncoder encoderOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return encodeV1;
    case ColorFormat::V8: return encodeV8;
    case ColorFormat::RGB24: return encodeRgb24;
    case ColorFormat::RGBA32: return encodeRgba32;
    case ColorFormat::ARGB32: return encodeArgb32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

Image::Image(size_t w, size_t h, ColorFormat format)
    : contentSize{divCeil(w * h * bitSizeOf(format), 8u)}
    , content{std::make_unique<u8[]>(contentSize)}
    , w{w}
    , h{h}
    , bitsPerPixel{bitSizeOf(format)}
    , f{format}
    , encoder{encoderOf(format)}
{
}

}  // namespace voxelio
