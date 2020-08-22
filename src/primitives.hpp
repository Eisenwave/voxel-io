#ifndef VXIO_PRIMITIVES_HPP
#define VXIO_PRIMITIVES_HPP
/*
 * primitives.hpp
 * --------------
 * Provides convenient aliases for C++ primitive types such as "u64" for "std::uint64_t".
 * Also contains key assertions about the available numeric types, such as a two's complement representation.
 *
 */

#include <cstdint>
#include <cstddef>
#include <limits>
#include <cstdio>

namespace voxelio {

// STATIC ASSERTSIONS FOR ALL TYPES ====================================================================================

static_assert(sizeof(std::uint8_t) == 1, "u8 must be one char in size");
static_assert(sizeof(float) == 4, "float must be a 32-bit floating point number");
static_assert(sizeof(double) == 8, "double must be a 64-bit floating point number");

static_assert(static_cast<signed char>(128) == -128, "expected two's complement platform");

static_assert(std::numeric_limits<float>::is_iec559, "voxelio depends on IEC 559 floats");
static_assert(std::numeric_limits<double>::is_iec559, "voxelio depends on IEC 559 doubles");

// PRIMITIVE ALIASES ===================================================================================================

/*
 * voxel-io uses u8 (aka. std::uint8_t) for all I/O-related classes and operations.
 * std::byte provides an inferior alternative which does not support all integer arithmetic.
 * Proponents of std::byte arguments that bytes should not be interpreted as numeric types, but it interprets them as
 * numbers itself, as leftshifts rely on ordering bytes by their significance inside a number.
 * std::byte supports operations like << which is defined as multiplying with pow(2, shift) mod N.
 *
 * In conclusion, there is no reason to ever use std::byte.
 * It's not logically consistent and provides zero utility.
 */

/// Unsigned 8-bit integer.
using u8 = std::uint8_t;
/// Unsigned 16-bit integer.
using u16 = std::uint16_t;
/// Unsigned 32-bit integer.
using u32 = std::uint32_t;
/// Unsigned 64-bit integer.
using u64 = std::uint64_t;
/// Largest unsigned integer.
using umax = std::uintmax_t;
/// Unsigned integer which can represent array sizes. Usually 64-bit on 64-bit platforms, else 32-bit.
using usize = std::size_t;

/// Signed 8-bit integer.
using i8 = std::int8_t;
/// Signed 16-bit integer.
using i16 = std::int16_t;
/// Signed 32-bit integer.
using i32 = std::int32_t;
/// Signed 64-bit integer.
using i64 = std::int64_t;
/// Largest signed integer.
using imax = std::intmax_t;

/// 32-bit floating point type.
using f32 = float;
/// 64-bit floating point type.
using f64 = double;
/// Largest floating point type.
using fmax = long double;

// OTHER ALIASES =======================================================================================================

/** @brief Integer representing a 32-bit color.
 * A (alpha) is the most significant byte.
 * B (blue) is the least significant byte.
 */
using argb32 = u32;

/// Alias for C-file pointers.
using cfile = std::FILE *;

}

#endif // PRIMITIVES_HPP
