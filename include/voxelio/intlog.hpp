#ifndef VXIO_INTLOG_HPP
#define VXIO_INTLOG_HPP
/*
 * intlog.hpp
 * -----------
 * Implements arithmetic related to integer logarithms or exponentiation.
 */

#include "bitcount.hpp"
#include "builtin.hpp"
#include "primitives.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace voxelio {

// POWER OF 2 TESTING ==================================================================================================

/**
 * @brief Returns whether an unsigned integer is a power of 2 or zero.
 * Note that this test is faster than having to test if val is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2 or if val is zero
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr bool isPow2or0(Uint val)
{
    return (val & (val - 1)) == 0;
}

/**
 * @brief Returns whether an unsigned integer is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2
 * @see is_pow2_or_zero
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr bool isPow2(Uint val)
{
    return val != 0 && isPow2or0(val);
}

// POWER OF 2 ROUNDING =================================================================================================

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2floor_naive(Uint val) noexcept;

/**
 * @brief Rounds up an unsigned integer to the next power of 2, minus 1.
 * 0 underflows to ~0.
 * Examples: 100 -> 127, 1 -> 1, 3 -> 3, 3000 -> 4095, 64 -> 127
 * @param v the value to round up
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint ceilPow2m1(Uint v)
{
    constexpr usize iterations = log2floor_naive(bits_v<Uint>);
    for (usize i = 0; i < iterations; ++i) {
        // after all iterations, all bits right to the msb will be filled with 1
        v |= v >> (1 << i);
    }
    return v;
}

/**
 * @brief Rounds up an unsigned integer to the next power of 2.
 * Powers of two are not affected.
 * 0 is not a power of 2 but treated as such and not rounded up.
 * Examples: 100 -> 128, 1 -> 1, 3 -> 4, 3000 -> 4096
 * @param v the value to round up
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint ceilPow2(Uint v)
{
    return ceilPow2m1(v - 1) + 1;
}

/**
 * @brief Rounds down an unsigned integer to the next power of 2.
 * Powers of 2 are not affected.
 * 0 can't be rounded down and is not a power of 2, so it is rounded up to 1 instead.
 * Examples: 100 -> 64, 1 -> 1, 3 -> 2, 3000 -> 2048
 * @param v the value to round down
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint floorPow2(Uint v)
{
    return ceilPow2m1(v >> 1) + 1;
}

// BASE 2 LOGARITHMS ===================================================================================================

/**
 * @brief Naive implementation of log2 using repeated single-bit rightshifting.
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int>>
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
    constexpr usize iterations = log2floor_naive(bits_v<Uint>);
    unsigned result = 0;

    for (usize i = iterations; i != 0; --i) {
        unsigned compBits = 1 << (i - 1);
        Uint compShift = Uint{1} << compBits;
        unsigned shift = unsigned{v >= compShift} << (i - 1);
        v >>= shift;
        result |= shift;
    }

    return result;
}

namespace detail {

constexpr u32 MultiplyDeBruijnBitPosition[32] = {0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
                                                 8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

}

/**
 * @brief log2floor implementation using De Bruijn multiplication.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn.
 * @param val the value
 */
constexpr u32 log2floor_debruijn(u32 val) noexcept
{
    constexpr u32 magic = 0x07C4ACDD;

    val = ceilPow2m1(val);
    val *= magic;
    val >>= 27;

    return detail::MultiplyDeBruijnBitPosition[val];
}

#ifdef VXIO_HAS_BUILTIN_CLZ
/**
 * @brief log2floor implementation using the builtin::countLeadingZeros intrinsic.
 * When available, this is by far the fastest implementation as it only requires the use of a clz instruction plus
 * some surrounding code.
 *
 * countLeadingZeros(0) is undefined and must be handled specially.
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
inline Uint log2floor_builtin(Uint v) noexcept
{
    constexpr int maxIndex = bits_v<Uint> - 1;
    return static_cast<Uint>(v == 0 ? 0 : maxIndex - builtin::countLeadingZeros(v));
}
#endif

/**
 * @brief Computes the floored binary logarithm of a given integer.
 * Example: log2floor(123) = 6
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It is undefined for negative values.
 *
 * Unlike a traditional log function, it is defined for 0: log2floor(0) = 0
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr Int log2floor(Int v) noexcept
{
#ifdef VXIO_HAS_BUILTIN_CLZ
    if (not isConstantEvaluated()) {
        return static_cast<Int>(log2floor_builtin(v));
    }
#endif
    if constexpr (std::is_same_v<Int, u32>) {
        return log2floor_debruijn(v);
    }
    else {
        return log2floor_fast<Int>(v);
    }
}

/**
 * @brief Computes the ceiled binary logarithm of a given integer.
 * Example: log2ceil(123) = 7
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It is undefined for negative values.
 *
 * Unlike a traditional log function, it is defined for 0: log2ceil(0) = 0
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr Int log2ceil(Int val) noexcept
{
    const Int result = log2floor(val);
    return result + not isPow2or0(val);
}

/**
 * @brief Computes the number of bits required to represent a given number.
 * Examples: bitLength(0) = 1, bitLength(3) = 2, bitLength(123) = 7, bitLength(4) = 3
 */
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr Int bitCount(Int val) noexcept
{
    return log2floor(val) + 1;
}

// ARBITRARY BASE LOGARITHMS ===========================================================================================

/**
 * @brief Naive implementation of log base N using repeated division.
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint logFloor_naive(Uint val, unsigned base) noexcept
{
    Uint result = 0;
    while (val /= base) {
        ++result;
    }
    return result;
}

/**
 * @brief The maximum possible exponent for a given base that can still be represented by a given integer type.
 * Example: maxExp<uint8_t, 10> = 2, because 10^2 is representable by an 8-bit unsigned integer but 10^3 isn't.
 */
template <typename Uint, unsigned BASE, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint maxExp = logFloor_naive<Uint>(static_cast<Uint>(~Uint{0u}), BASE);

static_assert(maxExp<uint8_t, 10> == 2);
static_assert(maxExp<u16, 10> == 4);
static_assert(maxExp<u32, 10> == 9);

/**
 * @brief Simple implementation of log base N using repeated multiplication.
 * This method is slightly more sophisticated than logFloor_naive because it avoids division.
 */
template <unsigned BASE, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint logFloor_simple(Uint val) noexcept
{
    constexpr Uint limit = maxExp<Uint, BASE>;

    Uint i = 0;
    Uint pow = BASE;
    for (; i <= limit; ++i, pow *= BASE) {
        if (val < pow) {
            return i;
        }
    }
    return i;
}

namespace detail {

/**
 * @brief Tiny array implementation to avoid including <array>.
 */
template <typename T, usize N, std::enable_if_t<N != 0, int> = 0>
struct Table {
    static constexpr usize size = N;
    T data[N];

    constexpr T front() const noexcept
    {
        return data[0];
    }

    constexpr T back() const noexcept
    {
        return data[N - 1];
    }

    constexpr T &operator[](usize i) noexcept
    {
        return data[i];
    }

    constexpr const T &operator[](usize i) const noexcept
    {
        return data[i];
    }
};

template <typename Uint, usize BASE>
constexpr Table<uint8_t, bits_v<Uint>> makeGuessTable()
{
    using resultType = decltype(makeGuessTable<Uint, BASE>());

    resultType result{};
    for (usize i = 0; i < resultType::size; ++i) {
        const Uint pow2 = static_cast<Uint>(Uint{1} << i);
        result[i] = logFloor_naive(pow2, BASE);
    }
    return result;
}

template <typename Uint, usize BASE, typename TableUint = nextLargerUintType<Uint>>
constexpr Table<TableUint, maxExp<Uint, BASE> + 2> makePowerTable()
{
    // the size of the table is maxPow<Uint, BASE> + 2 because we need to store the maximum power
    // +1 because we need to contain it, we are dealing with a size, not an index
    // +1 again because for narrow integers, we access one beyond the "end" of the table
    //
    // as a result, the last multiplication with BASE might overflow, but this is perfectly normal
    using resultType = decltype(makePowerTable<Uint, BASE, TableUint>());

    resultType result{};
    umax x = 1;
    for (usize i = 0; i < resultType::size; ++i, x *= BASE) {
        result[i] = static_cast<TableUint>(x);
    }
    return result;
}

}  // namespace detail

/**
 * @brief Computes pow(BASE, exponent) where BASE is known at compile-time.
 */
template <usize BASE, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint powConst(Uint exponent)
{
    if constexpr (isPow2(BASE)) {
        return 1 << (exponent * log2floor(BASE));
    }
    else {
        constexpr auto powers = detail::makePowerTable<Uint, BASE, Uint>();
        return static_cast<Uint>(powers.data[exponent]);
    }
}

namespace detail {

// table that maps from log_2(val) -> approximate log_N(val)
template <typename Uint, usize BASE>
constexpr auto logFloor_guesses = detail::makeGuessTable<Uint, BASE>();

// table that maps from log_N(val) -> pow(N, val + 1)
template <typename Uint, usize BASE>
constexpr auto logFloor_powers = detail::makePowerTable<Uint, BASE>();

}  // namespace detail

/**
 * @brief Computes the floored logarithm of a number with a given base.
 *
 * Examples:
 *     logFloor<10>(0) = 0
 *     logFloor<10>(5) = 0
 *     logFloor<10>(10) = 1
 *     logFloor<10>(123) = 2
 *
 * Note that unlike a traditional logarithm function, it is defined for 0 and is equal to 0.
 *
 * @see https://stackoverflow.com/q/63411054
 * @tparam BASE the base (e.g. 10)
 * @param val the input value
 * @return floor(log(val, BASE))
 */
template <usize BASE = 10, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint logFloor(Uint val) noexcept
{
    if constexpr (isPow2(BASE)) {
        return log2floor(val) / log2floor(BASE);
    }
    else {
        constexpr auto &guesses = detail::logFloor_guesses<Uint, BASE>;
        constexpr auto &powers = detail::logFloor_powers<Uint, BASE>;

        const u8 guess = guesses[log2floor(val)];

        if constexpr (sizeof(Uint) < sizeof(u64) || guesses.back() + 2 < powers.size) {
            return guess + (val >= powers[guess + 1]);
        }
        else {
            return guess + (val / BASE >= powers[guess]);
        }
    }
}

/**
 * @brief Convenience function that forwards to logFloor<10>.
 */
template <typename Uint>
constexpr Uint log10floor(Uint val)
{
    return logFloor<10, Uint>(val);
}

/**
 * @brief Computes the number of digits required to represent a number with a given base.
 */
template <usize BASE = 10, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint digitCount(Uint val) noexcept
{
    return logFloor<BASE>(val) + 1;
}

}  // namespace voxelio

#endif
