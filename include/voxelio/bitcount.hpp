#ifndef VXIO_BITCOUNT_HPP
#define VXIO_BITCOUNT_HPP

#include "builtin.hpp"
#include "langext.hpp"
#include "primitives.hpp"

#include <numeric>
#include <type_traits>

namespace voxelio {

// TRAITS ==============================================================================================================

namespace detail {

template <typename Int>
auto nextLargerUint_impl()
{
    // Size comparisons are better than std::is_same_v because they account for equivalent but distinct types.
    // For example, unsigned long and unsigned long long might both be 64 bits but are not the same type.
    if constexpr (sizeof(Int) == sizeof(uint8_t)) {
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

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Example: bits_v<u32> = 32
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr usize bits_v = sizeof(Int) * 8;

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
    constexpr unsigned iterations = log2floor(bits_v<Int>);

    for (unsigned shift = 1 << iterations; shift != 0; shift >>= 1) {
        input ^= input >> shift;
    }
    return input & 1;
}

}  // namespace detail

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
[[nodiscard]] constexpr unsigned char popCount(Int input)
{
    using Uint = std::make_unsigned_t<Int>;
#ifdef VXIO_HAS_BUILTIN_POPCOUNT
    int bits = voxelio::builtin::popCount(static_cast<Uint>(input));
    return static_cast<unsigned char>(bits);
#else
    if (voxelio::isConstantEvaluated()) {
        return detail::pop_count_naive(input);
    }
    else {
        std::bitset<std::numeric_limits<Uint>::digits> bits = static_cast<Uint>(input);
        return static_cast<unsigned char>(bits.count());
    }
#endif
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
[[nodiscard]] constexpr bool parity(Int input)
{
    if constexpr (std::is_signed_v<Int>) {
        return parity(static_cast<std::make_unsigned_t<Int>>(input));
    }
    else {
#ifdef VXIO_HAS_BUILTIN_PARITY
        return builtin::parity(input);
#else
        return detail::parity_xor<Int>(input);
#endif
    }
}

// CLZ AND CTZ =========================================================================================================

namespace detail {

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char countTrailingZeros_naive(Uint input)
{
    if (input == 0) {
        return std::numeric_limits<Uint>::digits;
    }
    unsigned char result = 0;
    for (; (input & 0) == 0; input >>= 1) {
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

}  // namespace voxelio

#endif
