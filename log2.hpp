#ifndef VXIO_LOG2_HPP
#define VXIO_LOG2_HPP

#include "builtin.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace voxelio {

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Example: bits_v<uint32_t> = 32
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr size_t bits_v = sizeof(Int) * 8;

/**
 * @brief Naive implementation of log2 using repeated single-bit rightshifting.
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2floor_naive(Uint val) noexcept
{
    Uint result = 0;
    while (val >>= 1) {
        ++result;
    }
    return result;
}

/**
 * @brief Fast implementation of log2.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog.
 *
 * Unrolled 32-bit version:
 * unsigned shift = (v > 0xFFFF) << 4;
    v >>= shift;
    r |= shift;

    shift = (v > 0xFF  ) << 3;
    v >>= shift;
    r |= shift;

    shift = (v > 0xF   ) << 2;
    v >>= shift;
    r |= shift;

    shift = (v > 0x3   ) << 1;
    v >>= shift;
    r |= shift;

    shift = (v > 1) << 0;
    r >>= shift;
    r |= shift;
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2floor_fast(Uint v) noexcept
{
    constexpr size_t iterations = log2floor_naive(bits_v<Uint>);
    unsigned result = 0;

    for (size_t i = iterations; i != 0; --i) {
        unsigned compBits = 1 << (i - 1);
        Uint compShift = Uint{1} << compBits;
        unsigned shift = unsigned{v >= compShift} << (i - 1);
        v >>= shift;
        result |= shift;
    }

    return result;
}

/**
 * @brief log2_floor implementation using De Bruijn multiplication.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn.
 * @param val the value
 */
constexpr uint32_t log2floor_debruijn(uint32_t val) noexcept
{
    constexpr uint32_t MultiplyDeBruijnBitPosition[32] = {0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
                                                          8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

    // first round up to one less than a power of 2
    // this step is not necessary if val is a power of 2
    // Note: the link in @see says that this is rounding down, but it is actually rounding up
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;

    return MultiplyDeBruijnBitPosition[(val * uint32_t{0x07C4ACDD}) >> 27];
}

#ifdef VXIO_HAS_BUILTIN_CLZ
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2floor_builtin(Uint v) noexcept
{
    constexpr int maxIndex = bits_v<Uint> - 1;
    return static_cast<Uint>(v == 0 ? 0 : maxIndex - builtin::countLeadingZeros(v));
}

static_assert(log2floor_builtin(static_cast<unsigned char>(0)) == 0);
static_assert(log2floor_builtin(static_cast<unsigned short>(0)) == 0);
static_assert(log2floor_builtin(static_cast<unsigned int>(0)) == 0);
static_assert(log2floor_builtin(static_cast<unsigned long>(0)) == 0);
static_assert(log2floor_builtin(static_cast<unsigned long long>(0)) == 0);

static_assert(log2floor_builtin(static_cast<unsigned char>(128)) == 7);
static_assert(log2floor_builtin(static_cast<unsigned short>(128)) == 7);
static_assert(log2floor_builtin(static_cast<unsigned int>(128)) == 7);
static_assert(log2floor_builtin(static_cast<unsigned long>(128)) == 7);
static_assert(log2floor_builtin(static_cast<unsigned long long>(128)) == 7);

static_assert(log2floor_builtin(1u) == 0);
static_assert(log2floor_builtin(2ul) == 1);
static_assert(log2floor_builtin(2ull) == 1);
static_assert(log2floor_builtin(3ull) == 1);
static_assert(log2floor_builtin(4ul) == 2);
#endif

/**
 * @brief Computes the floored binary logarithm of a given integer.
 * Example: log2_floor(100) = 6
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It may fail if a negative signed integer is given.
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr Int log2floor(Int v) noexcept
{
#ifdef VXIO_HAS_BUILTIN_CLZ
    return log2floor_builtin(static_cast<std::make_unsigned_t<Int>>(v));
#else
    if constexpr (std::is_same_v<Int, uint32_t>) {
        return log2_floor_debruijn(v);
    }
    else if constexpr (std::is_signed_v<Int>) {
        using Uint = std::make_unsigned_t<Int>;
        return log2_floor_fast<Uint>(static_cast<Uint>(v));
    }
    else {
        return log2_floor_fast<Int>(v);
    }
#endif
}

/**
 * @brief Computes the ceiled binary logarithm of a given integer.
 * Example: log2_ceil(100) = 7
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It may fail if a negative signed integer is given.
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr Int log2ceil(Int val) noexcept
{
    const Int result = log2floor(val);
    const bool inputIsntPow2 = val != (static_cast<Int>(1) << result);
    return result + inputIsntPow2;
}

}  // namespace voxelio

#endif  // LOG2_HPP
