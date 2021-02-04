#ifndef VXIO_BITCOUNT_HPP
#define VXIO_BITCOUNT_HPP

#include "builtin.hpp"
#include "langext.hpp"
#include "primitives.hpp"

#include <numeric>
#include <type_traits>

namespace voxelio {

// CLZ AND CTZ =========================================================================================================

namespace detail {

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char countTrailingZeros_naive(Uint input)
{
    if (input == 0) {
        return std::numeric_limits<Uint>::digits;
    }
    unsigned char result = 0;
    for (; (input & 1) == 0; input >>= 1) {
        ++result;
    }
    return result;
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char countLeadingZeros_naive(Uint input)
{
    constexpr Uint highestBit = Uint{1} << (std::numeric_limits<Uint>::digits - 1);

    if (input == 0) {
        return std::numeric_limits<Uint>::digits;
    }
    unsigned char result = 0;
    for (; (input & highestBit) == 0; input <<= 1) {
        ++result;
    }
    return result;
}

}  // namespace detail

/**
 * @brief Counts the number of leading zeros in a number.
 * The number of leading zeros in 0 is equal to the number of bits of the input type.
 * Example: countLeadingZeros(u8{7}) = 5
 */
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
[[nodiscard]] constexpr unsigned char countLeadingZeros(Int input)
{
#ifdef VXIO_HAS_BUILTIN_CLZ
    if (isConstantEvaluated()) {
        return detail::countLeadingZeros_naive(input);
    }
    return input == 0 ? std::numeric_limits<Int>::digits
                      : static_cast<unsigned char>(builtin::countLeadingZeros(input));
#else
    return detail::countLeadingZeros_naive(input);
#endif
}

/**
 * @brief Counts the number of trailing zeros in a number.
 * The number of trailing zeros in 0 is equal to the number of bits of the input type.
 * Example: countTrailingZeros(u8{8}) = 3
 */
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
[[nodiscard]] constexpr unsigned char countTrailingZeros(Int input)
{
#ifdef VXIO_HAS_BUILTIN_CTZ
    if (isConstantEvaluated()) {
        return detail::countTrailingZeros_naive(input);
    }
    return input == 0 ? std::numeric_limits<Int>::digits
                      : static_cast<unsigned char>(builtin::countTrailingZeros(input));
#else
    return detail::countTrailingZeros_naive(input);
#endif
}

// TYPE BIT COUNTS =====================================================================================================

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Example: bits_v<u32> = 32
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr usize bits_v = sizeof(Int) * 8;

/**
 * @brief templated variable which contains the log2 of the number of bits of a type.
 * Example: log2bits_v<u32> = 5
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
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

template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
using nextLargerUintType = decltype(detail::nextLargerUint_impl<Int>());

// BIT COUNTING ========================================================================================================

namespace detail {

// bitset operations are not constexpr so we need a naive implementations for constexpr contexts

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char popCount_naive(Uint input)
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
template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr bool parity_xor(Int input)
{
    constexpr unsigned iterations = log2bits_v<Int> - 1;

    for (unsigned shift = 1 << iterations; shift != 0; shift >>= 1) {
        input ^= input >> shift;
    }
    return input & 1;
}

}  // namespace detail

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
[[nodiscard]] constexpr unsigned char popCount(Int input)
{
    if constexpr (std::is_signed_v<Int>) {
        return popCount(static_cast<std::make_unsigned_t<Int>>(input));
    }
    else {
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
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
[[nodiscard]] constexpr bool parity(Int input)
{
    if constexpr (std::is_signed_v<Int>) {
        return parity(static_cast<std::make_unsigned_t<Int>>(input));
    }
    else {
        if (isConstantEvaluated()) {
            return detail::parity_xor<Int>(input);
        }
#ifdef VXIO_HAS_BUILTIN_PARITY
        return builtin::parity(input);
#else
        return detail::parity_xor<Int>(input);
#endif
    }
}

// STATIC TESTS ========================================================================================================

// These tests are not meant to be a full test suite but rather ensure that all functions can be used in a constexpr
// context.
static_assert(bits_v<i8> == 8);
static_assert(bits_v<u64> == 64);
static_assert(log2bits_v<i8> == 3);
static_assert(log2bits_v<u64> == 6);
static_assert(popCount(0b1010) == 2);
static_assert(parity(0b1010) == 0);
static_assert(parity(1) == 1);

}  // namespace voxelio

#endif
