#ifndef VXIO_COLOR_HPP
#define VXIO_COLOR_HPP

#include "types.hpp"

namespace voxelio {

struct ArgbColor {
    enum Value : argb32 {
        WHITE = 0xffffffff,
        INVISIBLE_WHITE = 0x00ffffff,
    };
};

struct Color32 {
    u8 b, g, r, a;

    // CONSTRUCTORS

    constexpr Color32();
    constexpr Color32(argb32 argb);
    constexpr Color32(u8 r, u8 g, u8 b, u8 a = 0xFF);
    constexpr Color32(float r, float g, float b, float a = 1);
    constexpr Color32(const Color32 &) = default;
    constexpr Color32(Color32 &&) = default;

    constexpr Color32 &operator=(const Color32 &) = default;
    constexpr Color32 &operator=(Color32 &&) = default;

    constexpr Color32 noalpha()
    {
        return {r, g, b};
    }

    constexpr float af() const
    {
        return a / 255.f;
    }

    constexpr float rf() const
    {
        return r / 255.f;
    }

    constexpr float gf() const
    {
        return g / 255.f;
    }

    constexpr float bf() const
    {
        return b / 255.f;
    }

    constexpr bool isTransparent() const
    {
        return a != 0xff;
    }

    constexpr bool isSolid() const
    {
        return a == 0xff;
    }

    constexpr bool isVisible() const
    {
        return a != 0;
    }

    constexpr bool isInvisible() const
    {
        return a == 0;
    }

    constexpr argb32 argb() const
    {
        return (argb32{a} << 24) | (argb32{r} << 16) | (argb32{g} << 8) | (argb32{b} << 0);
    }

    // OPERATORS

    constexpr operator argb32() const
    {
        return argb();
    }
};

constexpr u8 alpha(argb32 argb)
{
    return static_cast<u8>(argb >> 24);
}

constexpr u8 red(argb32 argb)
{
    return static_cast<u8>(argb >> 16);
}

constexpr u8 green(argb32 argb)
{
    return static_cast<u8>(argb >> 8);
}

constexpr u8 blue(argb32 argb)
{
    return static_cast<u8>(argb >> 0);
}

constexpr u8 alpha(Color32 argb)
{
    return argb.a;
}

constexpr u8 red(Color32 argb)
{
    return argb.r;
}

constexpr u8 green(Color32 argb)
{
    return argb.g;
}

constexpr u8 blue(Color32 argb)
{
    return argb.b;
}

namespace detail {
template <typename Float>
constexpr Float clamp01(Float x) noexcept
{
    return std::min<Float>(std::max<Float>(x, 0), 1);
}
}  // namespace detail

constexpr Color32::Color32(u8 r, u8 g, u8 b, u8 a) :  b{b}, g{g}, r{r}, a{a} {}

constexpr Color32::Color32() : Color32{u8{0}, u8{0}, u8{0}, u8{0}} {}

constexpr Color32::Color32(argb32 argb) : Color32{red(argb), green(argb), blue(argb), alpha(argb)} {}

constexpr Color32::Color32(float r, float g, float b, float a)
    : Color32{static_cast<u8>(detail::clamp01(r) * 0xFF),
              static_cast<u8>(detail::clamp01(g) * 0xFF),
              static_cast<u8>(detail::clamp01(b) * 0xFF),
              static_cast<u8>(detail::clamp01(a) * 0xFF)}
{
}

constexpr Color32 operator|(const Color32 &rgb0, const Color32 &rgb1)
{
    return rgb0.argb() | rgb1.argb();
}

constexpr Color32 operator&(const Color32 &rgb0, const Color32 &rgb1)
{
    return rgb0.argb() & rgb1.argb();
}

constexpr Color32 operator==(const Color32 &rgb0, const Color32 &rgb1)
{
    return rgb0.argb() == rgb1.argb();
}

constexpr Color32 operator!=(const Color32 &rgb0, const Color32 &rgb1)
{
    return rgb0.argb() != rgb1.argb();
}

enum ArgbOrder { ARGB, RGBA, BGRA };

namespace detail {
struct ChannelOffsets {
    unsigned a;
    unsigned r;
    unsigned g;
    unsigned b;
};

constexpr ChannelOffsets byteShiftAmountsOf(ArgbOrder format)
{
    switch (format) {
    case ArgbOrder::ARGB: return {3, 2, 1, 0};
    case ArgbOrder::RGBA: return {0, 3, 2, 1};
    case ArgbOrder::BGRA: return {0, 1, 2, 3};
    }
}

constexpr ChannelOffsets operator*(ChannelOffsets offsets, unsigned scalar)
{
    return {offsets.a * scalar, offsets.r * scalar, offsets.g * scalar, offsets.b * scalar};
}
}  // namespace detail

/**
 * Converts the given rgb32_t integer to an array containing the alpha, red, green and blue components of the integer in
 * that order.
 * The order in which the source bytes are taken from the integer depends on the given format
 * I.e. RGBA means that the least significant octet becomes the alpha component and the most significant octet becomes
 * the red component.
 */
template <ArgbOrder format>
void encodeArgb(argb32 argb, u8 out[4])
{
    constexpr auto shifts = detail::byteShiftAmountsOf(format) * 8;
    out[0] = static_cast<u8>(argb >> shifts.a);
    out[1] = static_cast<u8>(argb >> shifts.r);
    out[2] = static_cast<u8>(argb >> shifts.g);
    out[3] = static_cast<u8>(argb >> shifts.b);
}

/**
 * Converts an array of {alpha, red, green, blue} channels to an integer in the given format.
 */
template <ArgbOrder format>
argb32 decodeArgb(u8 argb[4])
{
    constexpr auto shifts = detail::byteShiftAmountsOf(format) * 8;
    argb32 result = 0;
    result |= static_cast<argb32>(argb[0]) << shifts.a;
    result |= static_cast<argb32>(argb[1]) << shifts.r;
    result |= static_cast<argb32>(argb[2]) << shifts.g;
    result |= static_cast<argb32>(argb[3]) << shifts.b;
    return result;
}

template <ArgbOrder FROM, ArgbOrder TO>
argb32 reorderColor(argb32 rgb)
{
    if constexpr (FROM == TO) {
        return rgb;
    }
    else {
        u8 buffer[4];
        encodeArgb<FROM>(rgb, buffer);
        return decodeArgb<TO>(buffer);
    }
}

}  // namespace voxelio

#endif  // COLOR_HPP
