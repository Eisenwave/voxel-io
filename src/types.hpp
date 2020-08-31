#ifndef VXIO_TYPES_HPP
#define VXIO_TYPES_HPP
/*
 * types.hpp
 * -----------
 * Defines various voxelio types and statically asserts certain properties, such as iec559 floats, uint8_t being one
 * byte, etc.
 */

#include "primitives.hpp"
#include "vec.hpp"

#include <cstdint>

namespace voxelio {

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

enum class Axis : unsigned { X = 0, Y = 1, Z = 2 };

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
