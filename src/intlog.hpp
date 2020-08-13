#ifndef VXIO_INTLOG_HPP
#define VXIO_INTLOG_HPP

#include "builtin.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace voxelio {

// TRAITS ==============================================================================================================

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Example: bits_v<uint32_t> = 32
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr size_t bits_v = sizeof(Int) * 8;

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
    constexpr size_t iterations = log2floor_naive(bits_v<Uint>);
    for (size_t i = 0; i < iterations; ++i) {
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
    constexpr uint32_t magic = 0x07C4ACDD;
    constexpr uint32_t MultiplyDeBruijnBitPosition[32] = {0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
                                                          8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

    val = ceilPow2m1(val);
    val *= magic;
    val >>= 27;

    return MultiplyDeBruijnBitPosition[val];
}

#ifdef VXIO_HAS_BUILTIN_CLZ
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2floor_builtin(Uint v) noexcept
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
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr Int log2floor(Int v) noexcept
{
    using Uint = std::make_unsigned_t<Int>;
#ifdef VXIO_HAS_BUILTIN_CLZ
    return static_cast<Int>(log2floor_builtin(static_cast<Uint>(v)));
#else
    if constexpr (std::is_same_v<Int, uint32_t>) {
        return log2floor_debruijn(v);
    }
    else if constexpr (std::is_signed_v<Int>) {
        return log2floor_fast<Uint>(static_cast<Uint>(v));
    }
    else {
        return log2floor_fast<Int>(v);
    }
#endif
}

/**
 * @brief Computes the ceiled binary logarithm of a given integer.
 * Example: log2ceil(123) = 7
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It is undefined for negative values.
 *
 * Unlike a traditional log function, it is defined for 0: log2ceil(0) = 1.
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

/**
 * @brief Computes the number of bits required to represent a given number.
 * Examples: bitLength(0) = 1, bitLength(3) = 2, bitLength(123) = 7, bitLength(4) = 3
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
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

namespace detail {

template <typename Uint, unsigned BASE>
constexpr Uint maxPow = logFloor_naive<Uint>(static_cast<Uint>(~Uint{0u}), BASE);

static_assert(maxPow<uint8_t, 10> == 2);
static_assert(maxPow<uint16_t, 10> == 4);
static_assert(maxPow<uint32_t, 10> == 9);

template <typename T, size_t N>
struct Table {
    static constexpr size_t size = N;
    T data[N];
};

template <typename Uint, size_t BASE>
constexpr Table<Uint, bits_v<Uint>> makeGuessTable()
{
    using resultType = decltype(makeGuessTable<Uint, BASE>());

    resultType result{};
    for (size_t i = 0; i < resultType::size; ++i) {
        Uint pow2 = static_cast<Uint>(Uint{1} << i);
        result.data[i] = logFloor_naive(pow2, BASE);
    }
    return result;
}

template <typename Uint, size_t BASE>
constexpr Table<uintmax_t, maxPow<Uint, BASE> + 2> makePowerTable()
{
    using resultType = decltype(makePowerTable<Uint, BASE>());

    resultType result{};
    uintmax_t x = 1;
    for (size_t i = 0; i < resultType::size; ++i, x *= 10) {
        result.data[i] = x;
    }
    return result;
}

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
 * @tparam BASE the base (e.g. 10)
 * @param val the input value
 * @return floor(log(val, BASE))
 */
template <size_t BASE = 10, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint logFloor(Uint val) noexcept
{
    if constexpr (isPow2(BASE)) {
        return log2floor(val) / log2floor(BASE);
    }
    else {
        // table that maps from log_2(val) -> approximate log_N(val)
        constexpr auto guesses = detail::makeGuessTable<Uint, BASE>();
        // table that maps from log_N(val) -> pow(N, val + 1)
        constexpr auto powers = detail::makePowerTable<Uint, BASE>();
        static_assert(guesses.data[0] == 0);
        static_assert(guesses.data[1] == 0);

        // The strategy in the following lines is to make an initial guess based on the binary logarithm.
        // For instance, the guess for 10 base 10 would be:
        //     log_10(2^log_2(10))
        //   = log_10(8^3)
        //   = 0
        // Due to floored rounding, this is just a lower bound and might need to be incremented once.
        //
        // If our input value is >= pow(base, guess + 1), we have to increment it.
        // For example, in the case of 10 base 10 with a guess of 0:
        //     pow(base = 10, guess + 1)
        //   = pow(10, 0 + 1)
        //  <= (val = 10)
        //
        // Instead of computing pow(BASE, guess), we can construct a table for all possible guesses.
        // However, we must compare with guess + 1 and pow(10, guess + 1) might not always be representable.
        // For example pow(10, 3) = 10000 is not representible as an 8-bit integer but would be necessary to test if
        // e.g. 200 should be incremented.
        // This is why we construct a table of type uintmax_t to make sure that we can always represent integers.
        // We can't use this tactic for uintmax_t though, so in that case we need to perform a division by the base
        // instead of looking up the next value, which is equivalent but more expensive.

        Uint guess = guesses.data[log2floor(val)];

        if constexpr (std::is_same_v<uintmax_t, Uint>) {
            return guess + (val / BASE >= powers.data[guess]);
        }
        else {
            return guess + (val >= powers.data[guess + 1]);
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
template <size_t BASE = 10, typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr Int digitCount(Int val) noexcept
{
    return logFloor<BASE>(val) + 1;
}

}  // namespace voxelio

#endif
