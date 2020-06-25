#ifndef VXIO_IMAGE_HPP
#define VXIO_IMAGE_HPP

#include "color.hpp"
#include "types.hpp"

#include <cstddef>
#include <memory>

namespace voxelio {

enum class ColorFormat { V1, V8, RGB24, ARGB32, RGBA32 };

constexpr size_t channelCountOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 1;
    case ColorFormat::RGB24: return 3;
    case ColorFormat::ARGB32: return 4;
    case ColorFormat::RGBA32: return 4;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr size_t bitDepthOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 8;
    case ColorFormat::RGB24: return 8;
    case ColorFormat::ARGB32: return 8;
    case ColorFormat::RGBA32: return 8;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr size_t bitSizeOf(ColorFormat format)
{
    switch (format) {
    case ColorFormat::V1: return 1;
    case ColorFormat::V8: return 8;
    case ColorFormat::RGB24: return 24;
    case ColorFormat::ARGB32: return 32;
    case ColorFormat::RGBA32: return 32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

using RgbEncoder = void (*)(Color32 rgb, u8 *out, size_t bitOffset);

/**
 * @brief Rudimentary implementation of an in-memory image with support for multiple color formats.
 */
class Image {
private:
    size_t contentSize;
    std::unique_ptr<u8[]> content;
    size_t w;
    size_t h;
    size_t bitsPerPixel;
    ColorFormat f;
    RgbEncoder encoder;

public:
    Image(size_t w, size_t h, ColorFormat format);
    Image(Image &&) = default;
    Image &operator=(Image &&) = default;

    /**
     * @brief Sets the color of a pixel at a given location.
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @param color the color
     */
    void setPixel(size_t x, size_t y, Color32 color)
    {
        VXIO_DEBUG_ASSERT_LT(x, w);
        VXIO_DEBUG_ASSERT_LT(y, h);
        setData(bitIndexOf(x, y), color);
    }

    /**
     * @brief Sets the color of a pixel at a given location.
     * @param pixelIndex the index of the pixel
     * @param color the color
     */
    void setPixel(size_t pixelIndex, Color32 color)
    {
        VXIO_DEBUG_ASSERT_LT(pixelIndex, w * h);
        setData(pixelIndex * bitsPerPixel, color);
    }

    /**
     * @brief Inserts the color at an exact bit-location.
     * @param bitIndex the index of the bit in the image
     * @param color the color
     */
    void setData(size_t bitIndex, Color32 color)
    {
        const size_t byteIndex = bitIndex / 8;
        const size_t bitOffset = bitIndex % 8;
        VXIO_DEBUG_ASSERT_LT(byteIndex, contentSize);
        encoder(color, &content[byteIndex], bitOffset);
    }

    u8 *data()
    {
        return content.get();
    }

    const u8 *data() const
    {
        return content.get();
    }

    size_t width() const
    {
        return w;
    }

    size_t height() const
    {
        return h;
    }

    ColorFormat format() const
    {
        return f;
    }

    size_t pixelIndexOf(size_t x, size_t y) const
    {
        return y * w + x;
    }

    size_t bitIndexOf(size_t x, size_t y) const
    {
        return pixelIndexOf(x, y) * bitsPerPixel;
    }
};

}  // namespace voxelio

#endif  // VXIO_IMAGE_HPP
