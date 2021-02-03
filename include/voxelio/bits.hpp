#ifndef VXIO_BITS_HPP
#define VXIO_BITS_HPP
/*
 * bits.hpp
 * -----------
 * Provides various constexpr bit operations such as popCount (bit counting) or left/right rotation.
 */

#include "assert.hpp"
#include "util.hpp"

#include "intdiv.hpp"
#include "intlog.hpp"

#include <cstddef>
#include <cstdint>

#ifndef VXIO_HAS_BUILTIN_POPCOUNT
#include <bitset>
#endif

namespace voxelio {

// BIT GETTING / SETTING ===============================================================================================

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr bool getBit(Uint input, usize index)
{
    return (input >> index) & 1;
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint setBit(Uint input, usize index)
{
    return input | (1 << index);
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint setBit(Uint input, usize index, bool value)
{
    return input ^ ((getBit(input) ^ value) << index);
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint clearBit(Uint input, usize index)
{
    return input & ~(1 << index);
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint flipBit(Uint input, usize index)
{
    return input ^ (1 << index);
}

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
    constexpr Uint highestBit = 1 << (std::numeric_limits<Uint>::digits - 1);

    if (input == 0) {
        return std::numeric_limits<Uint>::digits;
    }
    unsigned char result = 0;
    for (; (input & highestBit) == 0; input <<= 1) {
        ++result;
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

template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
[[nodiscard]] constexpr unsigned char countLeadingZeros(Int input)
{
#ifdef VXIO_HAS_BUILTIN_CLZ
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
    return input == 0 ? std::numeric_limits<Int>::digits
                      : static_cast<unsigned char>(builtin::countTrailingZeros(input));
#else
    return detail::countTrailingZeros_naive(input);
#endif
}

// PORTABLE BYTE SWAP IMPLEMENTATION ===================================================================================

namespace detail {

/**
 * @brief Reverses the bytes of any integer.
 * This implementation does not use any builtin function.
 * @param integer the integer
 */
template <typename Int>
constexpr Int reverseBytes_shift(Int integer)
{
    // Compiler Explorer experiments have shown that we can't use encode/decode with different endians, as gcc
    // loses the ability to recognize the byte swap with bswap automatically.
    // Even for this implementation, gcc only recognizes it up to uint32_t, clang recognizes it for 64 bits too.
    // This is why the builtin variant is quite useful.
    u8 octets[sizeof(Int)]{};
    for (usize i = 0; i < sizeof(Int); ++i) {
        octets[i] = static_cast<u8>(integer >> (i * 8));
    }
    Int result = 0;
    for (usize i = 0; i < sizeof(Int); ++i) {
        usize shift = i * 8;
        result |= Int{octets[sizeof(Int) - i - 1]} << shift;
    }
    return result;
}

}  // namespace detail

/**
 * @brief Reverses the bytes of any integer.
 * This implementation uses builtin::byteSwap().
 * @param integer the integer
 */
template <typename Int>
[[nodiscard]] constexpr Int reverseBytes(Int integer)
{
    if constexpr (sizeof(Int) == 1) {
        return integer;
    }
    else if constexpr (std::is_signed_v<Int>) {
        auto unsignedSwapped = reverseBytes(static_cast<std::make_unsigned_t<Int>>(integer));
        return static_cast<Int>(unsignedSwapped);
    }
    else {
#ifdef VXIO_HAS_BUILTIN_BSWAP
        return builtin::byteSwap(integer);
#else
        return detail::reverseBytes_shift(integer);
#endif
    }
}

static_assert(reverseBytes(0x11223344) == 0x44332211);

// BIT ROTATION ========================================================================================================

namespace detail {

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint rotl_impl(Uint n, unsigned char rot)
{
    constexpr Uint mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint hi = n << rot;
    Uint lo = n >> (-rot & mask);
    return hi | lo;
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint rotr_impl(Uint n, unsigned char rot)
{
    const unsigned int mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint lo = n >> rot;
    Uint hi = n << (-rot & mask);
    return hi | lo;
}

}  // namespace detail

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint leftRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return voxelio::isConstantEvaluated() ? detail::rotl_impl(n, rot) : voxelio::builtin::rotl(n, rot);
#else
    return detail::rotl_impl(n, rot);
#endif
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint rightRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return voxelio::isConstantEvaluated() ? detail::rotr_impl(n, rot) : voxelio::builtin::rotr(n, rot);
#else
    return detail::rotr_impl(n, rot);
#endif
}

}  // namespace voxelio

#endif  // UTIL_BITS_HPP
