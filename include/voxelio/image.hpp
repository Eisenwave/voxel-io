#ifndef VXIO_IMAGE_HPP
#define VXIO_IMAGE_HPP
/*
 * image.hpp
 * -----------
 * Provides a minimalistic image implementation.
 */

#include "color.hpp"
#include "types.hpp"

#include <cstddef>
#include <memory>

namespace voxelio {

/**
 * @brief An enum listing various color formats.
 * Possible channels which may appear in the format name are V (value), R (red), G (green), B (blue), A (alpha).
 *
 * V (value) describes the brightness of a grayscale value.
 */
enum class ColorFormat { V1, V8, VA16, RGB24, ARGB32, RGBA32 };

enum class WrapMode { CLAMP, REPEAT };

/**
 * @brief Returns the number of channels in a color format.
 * @param format the color format.
 * @return the number of channels
 */
constexpr usize channelCountOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 1;
    case ColorFormat::VA16: return 2;
    case ColorFormat::RGB24: return 3;
    case ColorFormat::ARGB32: return 4;
    case ColorFormat::RGBA32: return 4;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

/**
 * @brief Returns the bit-depth of a color format.
 * The bit-depth is the number of bits in each channel.
 * @param format the format
 * @return the bit-depth of a color format
 */
constexpr usize bitDepthOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 8;
    case ColorFormat::VA16: return 8;
    case ColorFormat::RGB24: return 8;
    case ColorFormat::ARGB32: return 8;
    case ColorFormat::RGBA32: return 8;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

/**
 * @brief Returns the size of a color format in bits.
 * @param format the format
 * @return the size in bits
 */
constexpr usize bitSizeOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 8;
    case ColorFormat::VA16: return 16;
    case ColorFormat::RGB24: return 24;
    case ColorFormat::ARGB32: return 32;
    case ColorFormat::RGBA32: return 32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

namespace detail {
using RgbEncoder = void (*)(Color32 rgb, u8 *out, usize bitOffset);
using RgbDecoder = Color32 (*)(const u8 *in, usize bitOffset);
using UvFunction = float (*)(float);

inline float repeat(float x) noexcept
{
    float integral;
    float fraction = std::modf(x, &integral);
    fraction += fraction < 0;
    return fraction == 0 ? 1 : fraction;
}
}  // namespace detail

/**
 * @brief Rudimentary implementation of an in-memory image with support for multiple color formats.
 */
class Image {
private:
    usize contentSize;
    std::unique_ptr<u8[]> content;
    usize w;
    usize h;
    usize bitsPerPixel;
    ColorFormat f;
    // poor-man's vtable
    detail::RgbEncoder encoder = nullptr;
    detail::RgbDecoder decoder = nullptr;
    detail::UvFunction uvFunction = nullptr;

public:
    Image(usize w, usize h, ColorFormat format, std::unique_ptr<u8[]> content, WrapMode = WrapMode::REPEAT);
    Image(usize w, usize h, ColorFormat format, WrapMode = WrapMode::REPEAT);

    Image(const Image &) = delete;
    Image(Image &&) = default;

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = default;

    u8 *data()
    {
        return content.get();
    }

    const u8 *data() const
    {
        return content.get();
    }

    usize dataSize() const
    {
        return contentSize;
    }

    ColorFormat format() const
    {
        return f;
    }

    usize width() const
    {
        return w;
    }

    usize height() const
    {
        return h;
    }

    void setWrapMode(WrapMode mode)
    {
        uvFunction = mode == WrapMode::CLAMP ? detail::clamp01<float> : detail::repeat;
    }

    Vec<usize, 2> uvToXy(Vec2f uv) const
    {
        usize x = static_cast<usize>(uvFunction(uv.x()) * float(w - 1));
        usize y = static_cast<usize>(uvFunction(uv.y()) * float(h - 1));
        return {x, y};
    }

    /**
     * @brief Returns the index of the pixel in memory.
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @return the pixel index
     */
    usize pixelIndexOf(usize x, usize y) const
    {
        VXIO_DEBUG_ASSERT_LT(x, w);
        VXIO_DEBUG_ASSERT_LT(y, h);
        return y * w + x;
    }

    /**
     * @brief Returns the color of a pixel at a given location.
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @return the color
     */
    Color32 getPixel(usize x, usize y) const
    {
        return decodeColor(bitIndexOf(x, y));
    }

    Color32 getPixel(Vec2f uv) const
    {
        Vec<usize, 2> xy = uvToXy(uv);
        return getPixel(xy.x(), xy.y());
    }

    /**
     * @brief Sets the color of a pixel at a given location.
     * @param pixelIndex the index of the pixel
     */
    Color32 getPixel(usize pixelIndex) const
    {
        VXIO_DEBUG_ASSERT_LT(pixelIndex, w * h);
        return decodeColor(pixelIndex * bitsPerPixel);
    }

    /**
     * @brief Sets the color of a pixel at a given location.
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @param color the color
     */
    void setPixel(usize x, usize y, Color32 color)
    {
        encodeColor(bitIndexOf(x, y), color);
    }

    /**
     * @brief Sets the color of a pixel at a given location.
     * @param pixelIndex the index of the pixel
     * @param color the color
     */
    void setPixel(usize pixelIndex, Color32 color)
    {
        VXIO_DEBUG_ASSERT_LT(pixelIndex, w * h);
        encodeColor(pixelIndex * bitsPerPixel, color);
    }

    void setPixel(Vec2f uv, Color32 color)
    {
        Vec<usize, 2> xy = uvToXy(uv);
        setPixel(xy.x(), xy.y(), color);
    }

private:
    usize bitIndexOf(usize x, usize y) const
    {
        return pixelIndexOf(x, y) * bitsPerPixel;
    }

    /**
     * @brief Returns the color at the given bit-location.
     * @param the index of the bit in the image
     * @return the color
     */
    Color32 decodeColor(usize bitIndex) const
    {
        const usize byteIndex = bitIndex / 8;
        const usize bitOffset = bitIndex % 8;
        VXIO_DEBUG_ASSERT_LT(byteIndex, contentSize);
        return decoder(content.get() + byteIndex, bitOffset);
    }

    /**
     * @brief Inserts the color at an exact bit-location.
     * @param bitIndex the index of the bit in the image
     * @param color the color
     */
    void encodeColor(usize bitIndex, Color32 color)
    {
        const usize byteIndex = bitIndex / 8;
        const usize bitOffset = bitIndex % 8;
        VXIO_DEBUG_ASSERT_LT(byteIndex, contentSize);
        encoder(color, content.get() + byteIndex, bitOffset);
    }

    static usize contentSizeOf(usize w, usize h, ColorFormat format);
};

}  // namespace voxelio

#endif  // VXIO_IMAGE_HPP
