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

// BIT COUNTING ========================================================================================================

namespace detail {

// bitset operations are not constexpr so we need a naive implementation for constexpr context
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char popCount_naive(Uint input)
{
    unsigned char result = 0;
    for (; input != 0; input >>= 1) {
        result += input & 1;
    }
    return result;
}

}  // namespace detail

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr unsigned char popCount(Int input)
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
constexpr Int reverseBytes(Int integer)
{
    if constexpr (sizeof(Int) == 1) {
        return integer;
    }
    else if constexpr (std::is_signed_v<Int>) {
        auto unsignedSwapped = builtin::byteSwap(static_cast<std::make_unsigned_t<Int>>(integer));
        return static_cast<Int>(unsignedSwapped);
    }
    else {
#ifdef VXIO_HAS_BUILTIN_BSWAP
        return builtin::byteSwap(integer);
#else
        return builtin::reverseBytes_naive(integer);
#endif
    }
}

static_assert(reverseBytes(0x11223344) == 0x44332211);

// BIT ROTATION ========================================================================================================

namespace detail {

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint rotl_impl(Uint n, unsigned char rot)
{
    constexpr Uint mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint hi = n << rot;
    Uint lo = n >> (-rot & mask);
    return hi | lo;
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint rotr_impl(Uint n, unsigned char rot)
{
    const unsigned int mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint lo = n >> rot;
    Uint hi = n << (-rot & mask);
    return hi | lo;
}

}  // namespace detail

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint leftRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return voxelio::isConstantEvaluated() ? detail::rotl_impl(n, rot) : voxelio::builtin::rotl(n, rot);
#else
    return detail::rotl_impl(n, rot);
#endif
}

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint rightRot(Uint n, unsigned char rot = 1)
{
#ifdef VXIO_HAS_BUILTIN_ROTL
    return voxelio::isConstantEvaluated() ? detail::rotr_impl(n, rot) : voxelio::builtin::rotr(n, rot);
#else
    return detail::rotr_impl(n, rot);
#endif
}

}  // namespace voxelio

#endif  // UTIL_BITS_HPP
