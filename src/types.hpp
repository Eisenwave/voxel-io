#ifndef VXIO_TYPES_HPP
#define VXIO_TYPES_HPP

#include "vec.hpp"

#include <cstdint>

namespace voxelio {

// STATIC ASSERTSIONS FOR ALL TYPES ====================================================================================

static_assert(sizeof(std::uint8_t) == 1, "u8 must be one char in size");
static_assert(sizeof(float) == 4, "float must be a 32-bit floating point number");
static_assert(sizeof(double) == 8, "double must be a 64-bit floating point number");
static_assert(std::numeric_limits<float>::is_iec559, "voxelio depends on IEC 559 floats");
static_assert(std::numeric_limits<double>::is_iec559, "voxelio depends on IEC 559 doubles");

// PRIMITIVE ALIASES ===================================================================================================

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using umax = std::uintmax_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using imax = std::intmax_t;

using f32 = float;
using f64 = double;
using fmax = long double;

// VEC ALIASES =========================================================================================================

using Vec2c = Vec<char, 2>;
using Vec2s = Vec<short, 2>;
using Vec2i = Vec<int, 2>;
using Vec2l = Vec<long, 2>;
using Vec2ll = Vec<long long, 2>;
using Vec2uc = Vec<unsigned char, 2>;
using Vec2us = Vec<unsigned short, 2>;
using Vec2ui = Vec<unsigned int, 2>;
using Vec2u = Vec<unsigned, 2>;
using Vec2ul = Vec<unsigned long, 2>;
using Vec2ull = Vec<unsigned long long, 2>;
using Vec2b = Vec<bool, 2>;
using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;
using Vec2ld = Vec<long double, 2>;
using Vec2i8 = Vec<i8, 2>;
using Vec2i16 = Vec<i16, 2>;
using Vec2i32 = Vec<i32, 2>;
using Vec2i64 = Vec<i64, 2>;
using Vec2imax = Vec<imax, 2>;
using Vec2u8 = Vec<u8, 2>;
using Vec2u16 = Vec<u16, 2>;
using Vec2u32 = Vec<u32, 2>;
using Vec2u64 = Vec<u64, 2>;
using Vec2umax = Vec<umax, 2>;
using Vec2size = Vec<std::size_t, 2>;

using Vec3c = Vec<char, 3>;
using Vec3s = Vec<short, 3>;
using Vec3i = Vec<int, 3>;
using Vec3l = Vec<long, 3>;
using Vec3ll = Vec<long long, 3>;
using Vec3uc = Vec<unsigned char, 3>;
using Vec3us = Vec<unsigned short, 3>;
using Vec3ui = Vec<unsigned int, 3>;
using Vec3u = Vec<unsigned, 3>;
using Vec3ul = Vec<unsigned long, 3>;
using Vec3ull = Vec<unsigned long long, 3>;
using Vec3b = Vec<bool, 3>;
using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;
using Vec3ld = Vec<long double, 3>;
using Vec3i8 = Vec<i8, 3>;
using Vec3i16 = Vec<i16, 3>;
using Vec3i32 = Vec<i32, 3>;
using Vec3i64 = Vec<i64, 3>;
using Vec3imax = Vec<imax, 3>;
using Vec3u8 = Vec<u8, 3>;
using Vec3u16 = Vec<u16, 3>;
using Vec3u32 = Vec<u32, 3>;
using Vec3u64 = Vec<u64, 3>;
using Vec3umax = Vec<umax, 3>;
using Vec3size = Vec<std::size_t, 3>;

// OTHER ALIASES =======================================================================================================

/** @brief Integer representing a 32-bit color.
 * A (alpha) is the most significant byte.
 * B (blue) is the least significant byte.
 */
using argb32 = u32;

using cfile = std::FILE *;

// ENUMERATIONS ========================================================================================================

using build::Endian;

constexpr const char *nameOf(Endian endian)
{
    switch (endian) {
    case Endian::BIG: return "BIG";
    case Endian::LITTLE: return "LITTLE";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

enum class Axis : unsigned { X, Y, Z };

constexpr const char *nameOf(Axis endian)
{
    switch (endian) {
    case Axis::X: return "X";
    case Axis::Y: return "Y";
    case Axis::Z: return "Z";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

// COMMON STRUCTS ======================================================================================================

/** @brief 64-bit generic voxel representation. */
struct Voxel64 {
    using pos_t = Vec3i64;

    /** 3x 64-bit position of the voxel. */
    pos_t pos;
    union {
        /** 32-bit color of the voxel. */
        argb32 argb;
        /** 32-bit index in a color palette. */
        u32 index;
    };
};

/** @brief 32-bit generic voxel representation. */
struct Voxel32 {
    using pos_t = Vec3i32;

    /** 3x 32-bit position of the voxel. */
    pos_t pos;
    union {
        /** 32-bit color of the voxel. */
        argb32 argb;
        /** 32-bit index in a color palette. */
        u32 index;
    };
};

// VOXEL UTILITIES =====================================================================================================

template <typename T>
constexpr bool is_voxel_v = std::is_same_v<T, Voxel32> || std::is_same_v<T, Voxel64>;

constexpr bool operator==(const voxelio::Voxel32 &lhs, const voxelio::Voxel32 &rhs)
{
    return lhs.pos == rhs.pos && lhs.argb == rhs.argb;
}

constexpr bool operator!=(const voxelio::Voxel32 &lhs, const voxelio::Voxel32 &rhs)
{
    return lhs.pos != rhs.pos || lhs.argb != rhs.argb;
}

constexpr bool operator==(const voxelio::Voxel64 &lhs, const voxelio::Voxel64 &rhs)
{
    return lhs.pos == rhs.pos && lhs.argb == rhs.argb;
}

constexpr bool operator!=(const voxelio::Voxel64 &lhs, const voxelio::Voxel64 &rhs)
{
    return lhs.pos != rhs.pos || lhs.argb != rhs.argb;
}

template <typename To, typename From, std::enable_if_t<(is_voxel_v<To> && is_voxel_v<From>), int> = 0>
constexpr To voxelCast(From from)
{
    if constexpr (std::is_same_v<To, From>) {
        return from;
    }
    else if constexpr (std::is_same_v<To, Voxel32>) {
        return Voxel32{static_vec_cast<i32>(from.pos), {from.argb}};
    }
    else {
        return Voxel64{static_vec_cast<i64>(from.pos), {from.argb}};
    }
}

static_assert(voxelCast<Voxel32>(Voxel64{{666, 0, 0}, {0xff}}) == Voxel32{{666, 0, 0}, {0xff}});

}  // namespace voxelio

#endif  // VXIO_TYPES_HPP
