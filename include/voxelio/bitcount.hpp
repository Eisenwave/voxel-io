#ifndef VXIO_BITCOUNT_HPP
#define VXIO_BITCOUNT_HPP

#include "builtin.hpp"
#include "langext.hpp"
#include "primitives.hpp"

#include <type_traits>

#ifndef VXIO_HAS_BUILTIN_POPCOUNT
#include <bitset>
#endif

#define VXIO_INTEGRAL_TYPENAME(T) typename T, ::std::enable_if_t<::std::is_integral_v<T>, int> = 0
#define VXIO_UNSIGNED_TYPENAME(T) typename T, ::std::enable_if_t<::std::is_unsigned_v<T>, int> = 0

namespace voxelio {

// TYPE BIT COUNTS =====================================================================================================

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Unlike std::numeric_limits<T>::digits, this simply considers the bit count, which is different from the digit count
 * for signed types.
 * Example: bits_v<u32> = 32
 */
template <VXIO_INTEGRAL_TYPENAME(Int)>
constexpr usize bits_v = sizeof(Int) * 8;

// ALTERNATING BIT SEQUENCE ============================================================================================

/**
 * @brief Creates an alternating sequence of 1s and 0s, starting with 1.
 *
 * Examples: alternate(1, 2) = 0b...0101010101
 *           alternate(1, 3) = 0b...1001001001
 *           alternate(2, 2) = 0b...1100110011
 *           alternate(2, 4) = 0b...1100000011
 *
 * @param period describes how many 1 and 0 bits per period should be stored, must not be zero
 * @param mod the total number of bits per period, before scaling by the period parameter. Only the first bit of each
 * unscaled period is a 1-bit. If mod == 1, then all bits are 1-bits. Must not be zero.
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint alternate(unsigned period = 1, unsigned mod = 2) noexcept
{
    Uint result = 0;
    for (usize i = 0; i < bits_v<Uint>; ++i) {
        Uint bit = ((i / period) % mod) == 0;
        result |= bit << i;
    }
    return result;
}

// CLZ AND CTZ =========================================================================================================

namespace detail {

template <typename Uint, unsigned MOD = 2>
static constexpr Uint ALTERNATING_MASKS[6] = {alternate<Uint>(1, MOD),
                                              alternate<Uint>(2, MOD),
                                              alternate<Uint>(4, MOD),
                                              alternate<Uint>(8, MOD),
                                              alternate<Uint>(16, MOD),
                                              alternate<Uint>(32, MOD)};

template <typename Uint>
constexpr unsigned char countLeadingZeros_naive(Uint input)
{
    constexpr Uint highestBit = Uint{1} << (bits_v<Uint> - 1);

    if (input == 0) {
        return bits_v<Uint>;
    }
    unsigned char result = 0;
    for (; (input & highestBit) == 0; input <<= 1) {
        ++result;
    }
    return result;
}

template <typename Uint>
constexpr unsigned char countTrailingZeros_naive(Uint input)
{
    if (input == 0) {
        return bits_v<Uint>;
    }
    unsigned char result = 0;
    for (; (input & 1) == 0; input >>= 1) {
        ++result;
    }
    return result;
}

template <typename Uint>
constexpr unsigned char findFirstSet_parallel(Uint input)
{
    static_assert((bits_v<Uint> & (bits_v<Uint> - 1)) == 0, "Expected bit count of Uint to be a power of 2");

    constexpr usize iterations = countTrailingZeros_naive(bits_v<Uint>);
    constexpr usize resultMask = bits_v<Uint> - 1;

    usize result = bits_v<Uint>;
    for (usize i = 0, add = 1; i < iterations; ++i, add <<= 1) {
        bool hasBits = input & ALTERNATING_MASKS<Uint>[i];
        result -= hasBits * add;
    }

    return static_cast<unsigned char>(result & resultMask);
}

template <typename Uint>
constexpr unsigned char countTrailingZeros_ffs(Uint input)
{
    Uint ffs = findFirstSet_parallel(input);
    return ffs == 0 ? bits_v<Uint> : ffs - 1;
}

}  // namespace detail

/**
 * @brief Counts the number of leading zeros in a number.
 * The number of leading zeros in 0 is equal to the number of bits of the input type.
 * Example: countLeadingZeros(u8{7}) = 5
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char countLeadingZeros(Uint input)
{
#ifdef VXIO_HAS_BUILTIN_CLZ
    if (isConstantEvaluated()) {
        return detail::countLeadingZeros_naive(input);
    }
    return input == 0 ? bits_v<Uint> : static_cast<unsigned char>(builtin::countLeadingZeros(input));
#else
    return detail::countLeadingZeros_naive(input);
#endif
}

/**
 * @brief Counts the number of trailing zeros in a number.
 * The number of trailing zeros in 0 is equal to the number of bits of the input type.
 * Example: countTrailingZeros(u8{8}) = 3
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char countTrailingZeros(Uint input)
{
#ifdef VXIO_HAS_BUILTIN_CTZ
    if (isConstantEvaluated()) {
        return detail::countTrailingZeros_naive(input);
    }
    return input == 0 ? bits_v<Uint> : static_cast<unsigned char>(builtin::countTrailingZeros(input));
#else
    return detail::countTrailingZeros_naive(input);
#endif
}

// LOG2 OF TYPE BIT COUNTS =============================================================================================

/**
 * @brief templated variable which contains the log2 of the number of bits of a type.
 * Example: log2bits_v<u32> = 5
 */
template <VXIO_INTEGRAL_TYPENAME(Int)>
constexpr usize log2bits_v = countTrailingZeros(bits_v<Int>);

// TRAITS ==============================================================================================================

namespace detail {

template <typename Int>
auto nextLargerUint_impl()
{
    // Size comparisons are better than std::is_same_v because they account for equivalent but distinct types.
    // For example, unsigned long and unsigned long long might both be 64 bits but are not the same type.
    if constexpr (sizeof(Int) == sizeof(u8)) {
        return u16{0};
    }
    else if constexpr (sizeof(Int) == sizeof(u16)) {
        return u32{0};
    }
    else if constexpr (sizeof(Int) == sizeof(u32)) {
        return u64{0};
    }
    else {
        return umax{0};
    }
}

}  // namespace detail

template <VXIO_UNSIGNED_TYPENAME(Uint)>
using nextLargerUintType = decltype(detail::nextLargerUint_impl<Uint>());

// BIT COUNTING ========================================================================================================

namespace detail {

// bitset operations are not constexpr so we need a naive implementations for constexpr contexts

template <typename Int>
constexpr unsigned char popCount_naive(Int input)
{
    unsigned char result = 0;
    for (; input != 0; input >>= 1) {
        result += input & 1;
    }
    return result;
}

// This implementation is taken from https://stackoverflow.com/a/21618038 .
// The core idea is to XOR the high half bits with the low half bits recursively.
// In the end, the lowest bit will have been XORed with every other bit.
template <typename Int>
constexpr bool parity_xor(Int input)
{
    constexpr unsigned iterations = log2bits_v<Int> - 1;

    for (unsigned shift = 1 << iterations; shift != 0; shift >>= 1) {
        input ^= input >> shift;
    }
    return input & 1;
}

}  // namespace detail

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char popCount(Uint input)
{
    if (isConstantEvaluated()) {
        return detail::popCount_naive(input);
    }
#ifdef VXIO_HAS_BUILTIN_POPCOUNT
    return static_cast<unsigned char>(builtin::popCount(input));
#else
    std::bitset<std::numeric_limits<Int>::digits> bits = input;
    return static_cast<unsigned char>(bits.count());
#endif
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr bool parity(Uint input)
{
    if (isConstantEvaluated()) {
        return detail::parity_xor<Uint>(input);
    }
#ifdef VXIO_HAS_BUILTIN_PARITY
    return builtin::parity(input);
#else
    return detail::parity_xor<Uint>(input);
#endif
}

// STATIC TESTS ========================================================================================================

// These tests are not meant to be a full test suite but rather ensure that all functions can be used in a constexpr
// context.
static_assert(bits_v<i8> == 8);
static_assert(bits_v<u64> == 64);
static_assert(log2bits_v<i8> == 3);
static_assert(log2bits_v<u64> == 6);
static_assert(popCount(0b1010u) == 2);
static_assert(parity(0b1010u) == 0);
static_assert(parity(1u) == 1);

}  // namespace voxelio

#endif
