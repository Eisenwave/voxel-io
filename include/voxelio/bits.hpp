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
        return isConstantEvaluated() ? detail::reverseBytes_shift(integer) : builtin::byteSwap(integer);
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

/**
 * @brief Rotates an integer to the left by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the low side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint leftRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return isConstantEvaluated() ? detail::rotl_impl(n, rot) : builtin::rotl(n, rot);
#else
    return detail::rotl_impl(n, rot);
#endif
}

/**
 * @brief Rotates an integer to the right by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the high side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
[[nodiscard]] constexpr Uint rightRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return isConstantEvaluated() ? detail::rotr_impl(n, rot) : builtin::rotr(n, rot);
#else
    return detail::rotr_impl(n, rot);
#endif
}

}  // namespace voxelio

#endif  // UTIL_BITS_HPP
