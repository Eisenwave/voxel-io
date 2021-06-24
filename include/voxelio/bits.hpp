#ifndef VXIO_BITS_HPP
#define VXIO_BITS_HPP
/*
 * bits.hpp
 * -----------
 * Provides various constexpr bit operations such as left/right rotation.
 */

#include "bitcount.hpp"
#include "intdiv.hpp"
#include "util.hpp"

#include <cstddef>
#include <cstdint>

#ifndef VXIO_HAS_BUILTIN_POPCOUNT
#include <bitset>
#endif

namespace voxelio {

// BIT GETTING / SETTING ===============================================================================================

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr bool getBit(Uint input, unsigned index) noexcept
{
    return (input >> index) & Uint{1};
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint clearBit(Uint input, unsigned index) noexcept
{
    return input & ~(Uint{1} << index);
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint flipBit(Uint input, unsigned index) noexcept
{
    return input ^ (Uint{1} << index);
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint setBit(Uint input, unsigned index) noexcept
{
    return input | (Uint{1} << index);
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint setBit(Uint input, unsigned index, bool value) noexcept
{
    return clearBit(input, index) | (Uint{value} << index);
}

// PORTABLE BYTE SWAP IMPLEMENTATION ===================================================================================

namespace detail {

template <typename Int>
constexpr Int reverseBytes_naive(Int integer) noexcept
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

template <typename Int>
constexpr Int reverseBits_shift(Int integer, const unsigned bitLimit = 0) noexcept
{
    constexpr unsigned start = log2bits_v<Int>;

    for (unsigned i = start; --i != (bitLimit - 1);) {
        Int lo = integer & ALTERNATING_MASKS<Int>[i];
        Int hi = integer & ~ALTERNATING_MASKS<Int>[i];

        lo <<= 1 << i;
        hi >>= 1 << i;

        integer = lo | hi;
    }

    return integer;
}

template <typename Int>
constexpr Int reverseBytes_shift(Int integer) noexcept
{
    return reverseBits_shift<Int>(integer, 3);
}

}  // namespace detail

/**
 * @brief Reverses the bytes of any integer.
 * This implementation uses builtin::byteSwap().
 * @param integer the integer
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint reverseBytes(Uint integer) noexcept
{
    if constexpr (sizeof(Uint) == 1) {
        return integer;
    }
    else {
#ifdef VXIO_HAS_BUILTIN_BSWAP
        return isConstantEvaluated() ? detail::reverseBytes_shift(integer) : builtin::byteSwap(integer);
#else
        return detail::reverseBytes_shift(integer);
#endif
    }
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint reverseBits(Uint integer) noexcept
{
#ifdef VXIO_HAS_BUILTIN_BITREV
    return isConstantEvaluated() ? detail::reverseBits_shift(integer) : builtin::bitReverse(integer);
#endif
    return detail::reverseBits_shift(integer);
}

// BIT ROTATION ========================================================================================================

namespace detail {

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint leftRot_shift(Uint n, unsigned char rot) noexcept
{
    constexpr unsigned char mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint hi = n << rot;
    Uint lo = n >> (-rot & mask);
    return hi | lo;
}

template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rightRot_shift(Uint n, unsigned char rot) noexcept
{
    constexpr unsigned char mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint lo = n >> rot;
    Uint hi = n << (-rot & mask);
    return hi | lo;
}

}  // namespace detail

/**
 * @brief Rotates an integer to the left by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the low side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint leftRot(Uint n, unsigned char rot = 1) noexcept
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return isConstantEvaluated() ? detail::leftRot_shift(n, rot) : builtin::rotateLeft(n, rot);
#else
    return detail::leftRot_shift(n, rot);
#endif
}

/**
 * @brief Rotates an integer to the right by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the high side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <VXIO_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rightRot(Uint n, unsigned char rot = 1) noexcept
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return isConstantEvaluated() ? detail::rightRot_shift(n, rot) : builtin::rotateRight(n, rot);
#else
    return detail::rightRot_shift(n, rot);
#endif
}

}  // namespace voxelio

#endif  // UTIL_BITS_HPP
