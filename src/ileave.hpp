#ifndef ILEAVE_HPP
#define ILEAVE_HPP
/*
 * ileave.hpp
 * -----------
 * Provides bit-interleaving and bit-de-interleaving functionality.
 * Each function typically has multiple implementations in detail:: namespaces, which are then chosen in a single
 * function in voxelio::
 */

#include "assert.hpp"
#include "bits.hpp"
#include "intdiv.hpp"
#include "intlog.hpp"
#include "primitives.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace voxelio {

// ZERO-BIT INTERLEAVING ===============================================================================================

namespace detail {

/**
 * @brief Interleaves an input number with <bits> zero-bits per input bit. Example: 0b11 -> 0b0101
 * @param input the input number
 * @param bits the amount of zero bits per input bit to interleave
 * @return the input number interleaved with input-bits
 */
constexpr u64 ileaveZeros_naive(u32 input, usize bits)
{
    const auto lim = std::min<usize>(32, voxelio::divCeil<usize>(64, bits + 1));

    u64 result = 0;
    for (usize i = 0, b_out = 0; i < lim; ++i) {
        result |= static_cast<u64>(input & 1) << b_out;
        input >>= 1;
        b_out += bits + 1;
    }

    return result;
}

/**
 * @brief Duplicates each input bit <bits> times. Example: 0b101 -> 0b110011
 * @param input the input number
 * @param out_bits_per_in_bits the output bits per input bits
 * @return the output number or 0 if the bits parameter was zero
 */
constexpr u64 duplBits_naive(u64 input, usize out_bits_per_in_bits)
{
    if (out_bits_per_in_bits == 0) {
        return 0;
    }
    const auto lim = voxelio::divCeil<usize>(64, out_bits_per_in_bits);

    u64 result = 0;
    for (usize i = 0, b_out = 0; i < lim; ++i) {
        for (usize j = 0; j < out_bits_per_in_bits; ++j, ++b_out) {
            result |= static_cast<u64>((input >> i) & 1) << b_out;
        }
    }

    return result;
}

}  // namespace detail

/**
 * @brief Interleaves <BITS> zero-bits inbetween each input bit.
 * @param input the input number
 * @tparam BITS the number of bits to be interleaved, must be > 0
 * @return the input interleaved with <BITS> bits
 */
template <usize BITS>
constexpr u64 ileaveZeros_const(u32 input)
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr u64 MASKS[] = {
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 1),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 2),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 4),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 8),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 16),
        };
        // log2_floor(0) == 0 so this is always safe, even for 1 bit
        constexpr int start = 4 - (voxelio::log2floor(BITS >> 1));

        u64 n = input;
        for (int i = start; i != -1; --i) {
            usize shift = BITS * (1 << i);
            n |= n << shift;
            n &= MASKS[i];
        }

        return n;
    }
}

// BITWISE DE-INTERLEAVING =============================================================================================

namespace detail {

/**
 * @brief Removes each interleaved <bits> bits. Example: 0b010101 --rem 1--> 0b111
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
constexpr u64 remIleavedBits_naive(u64 input, usize bits) noexcept
{
    // increment once to avoid modulo divisions by 0
    // this way our function is noexcept and safe for all inputs
    ++bits;
    u64 result = 0;
    for (usize i = 0, b_out = 0; i < 64; ++i) {
        if (i % bits == 0) {
            result |= (input & 1) << b_out++;
        }
        input >>= 1;
    }

    return result;
}

}  // namespace detail

/**
 * @brief Removes each interleaved <BITS> bits. Example: 0b010101 --rem 1--> 0b111
 * If BITS is zero, no bits are removed and the input is returned.
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
template <usize BITS>
constexpr u64 remIleavedBits_const(u64 input) noexcept
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr u64 MASKS[] = {
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 1),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 2),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 4),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 8),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 16),
            detail::duplBits_naive(detail::ileaveZeros_naive(~u32(0), BITS), 32),
        };
        // log2_floor(0) == 0 so this is always safe, even for 1 bit
        constexpr usize iterations = 5 - (voxelio::log2floor(BITS >> 1));

        input &= MASKS[0];

        for (usize i = 0; i < iterations; ++i) {
            usize rshift = (1 << i) * BITS;
            input |= input >> rshift;
            input &= MASKS[i + 1];
        }

        return input;
    }
}

// NUMBER INTERLEAVING =================================================================================================

/**
 * @brief Interleaves 2 integers, where hi comprises the upper bits of each bit pair and lo the lower bits.
 * Examle: ileave2(0b111, 0b000) = 0b101010
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param hi the high bits
 * @param lo the low bits
 * @return the interleaved bits
 */
constexpr u64 ileave2(u32 hi, u32 lo)
{
    constexpr u64 MASKS[] = {0x5555'5555'5555'5555,
                                  0x3333'3333'3333'3333,
                                  0x0F0F'0F0F'0F0F'0F0F,
                                  0x00FF'00FF'00FF'00FF,
                                  0x0000'FFFF'0000'FFFF};

    u64 result = 0;
    u32 *nums[] = {&hi, &lo};

    for (usize i = 0; i < 2; ++i) {
        u64 n = *nums[i];
        for (usize i = 4; i != std::numeric_limits<usize>::max(); --i) {
            n |= n << (1 << i);
            n &= MASKS[i];
        }
        result |= n << (1 - i);
    }

    return result;
}

namespace detail {

constexpr u64 ileave3_naive(u32 x, u32 y, u32 z)
{
    return (detail::ileaveZeros_naive(x, 2) << 2) | (detail::ileaveZeros_naive(y, 2) << 1) | ileaveZeros_naive(z, 2);
}

}  // namespace detail

/**
 * @brief Interleaves 3 integers, where x comprises the uppermost bits of each bit triple and z the lowermost bits.
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param x the highest bits
 * @param y the middle bits
 * @param z the lowest bits
 * @return the interleaved bits
 */
constexpr u64 ileave3(u32 x, u32 y, u32 z)
{
    return (ileaveZeros_const<2>(x) << 2) | (ileaveZeros_const<2>(y) << 1) | ileaveZeros_const<2>(z);
}

// NUMBER DE-INTERLEAVING ==============================================================================================

namespace detail {

constexpr void dileave3_naive(u64 n, u32 out[3])
{
    out[0] = static_cast<u32>(detail::remIleavedBits_naive(n >> 2, 2));
    out[1] = static_cast<u32>(detail::remIleavedBits_naive(n >> 1, 2));
    out[2] = static_cast<u32>(detail::remIleavedBits_naive(n >> 0, 2));
}

}  // namespace detail

/**
 * @brief Deinterleaves 3 integers which are interleaved in a single number.
 * Visualization: abcdefghi -> (adg, beh, cfi)
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param n the number
 * @return the interleaved bits
 */
constexpr void dileave3(u64 n, u32 out[3])
{
    out[0] = static_cast<u32>(remIleavedBits_const<2>(n >> 2));
    out[1] = static_cast<u32>(remIleavedBits_const<2>(n >> 1));
    out[2] = static_cast<u32>(remIleavedBits_const<2>(n >> 0));
}

// BYTE INTERLEAVING ===================================================================================================

/**
 * @brief Interleaves up to 8 bytes into a 64-bit integer.
 * @param bytes the bytes in little-endian order
 * @tparam COUNT the number of bytes to interleave
 * @return the interleaved bytes stored in a 64-bit integer
 */
template <usize COUNT, std::enable_if_t<COUNT <= 8, int> = 0>
constexpr u64 ileaveBytes_const(u64 bytes)
{
    u64 result = 0;
    // use if-constexpr to avoid instantiation of ileave_zeros
    if constexpr (COUNT != 0) {
        for (usize i = 0; i < COUNT; ++i) {
            result |= ileaveZeros_const<COUNT - 1>(bytes & 0xff) << i;
            bytes >>= 8;
        }
    }
    return result;
}

namespace detail {

// alternative implementation using a naive algorithm
constexpr u64 ileaveBytes_naive(u64 bytes, usize count)
{
    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    u64 result = 0;
    for (usize i = 0; i < count; ++i) {
        result |= ileaveZeros_naive(bytes & 0xff, count - 1) << i;
        bytes >>= 8;
    }
    return result;
}

// alternative implementation adapting ileave_bytes_const to work with a runtime parameter
constexpr u64 ileaveBytes_jmp(u64 bytes, usize count)
{
    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    switch (count) {
    case 0: return ileaveBytes_const<0>(bytes);
    case 1: return ileaveBytes_const<1>(bytes);
    case 2: return ileaveBytes_const<2>(bytes);
    case 3: return ileaveBytes_const<3>(bytes);
    case 4: return ileaveBytes_const<4>(bytes);
    case 5: return ileaveBytes_const<5>(bytes);
    case 6: return ileaveBytes_const<6>(bytes);
    case 7: return ileaveBytes_const<7>(bytes);
    case 8: return ileaveBytes_const<8>(bytes);
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

}  // namespace detail

/**
 * @brief Interleaves up to 8 bytes into a 64-bit integer.
 * @param bytes the bytes in little-endian order
 * @param count number of bytes to interleave
 * @return the interleaved bytes stored in a 64-bit integer
 */
constexpr u64 ileaveBytes(u64 bytes, usize count)
{
    return detail::ileaveBytes_jmp(bytes, count);
}

// BYTE DE-INTERLEAVING ================================================================================================

template <usize COUNT, std::enable_if_t<COUNT <= 8, int> = 0>
constexpr u64 dileaveBytes_const(u64 ileaved)
{
    u64 result = 0;
    // use if-constexpr to avoid instantiation of rem_ileaved_bits
    if constexpr (COUNT != 0) {
        for (usize i = 0; i < COUNT; ++i) {
            // if we also masked the result with 0xff, then this would be safe for a hi-polluted ileaved number
            result |= remIleavedBits_const<COUNT - 1>(ileaved >> i);
            result <<= 8;
        }
    }
    return result;
}

namespace detail {

// alternative implementation adapting ileave_bytes_const to work with a runtime parameter
constexpr u64 dileaveBytes_jmp(u64 bytes, usize count)
{
    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    switch (count) {
    case 0: return dileaveBytes_const<0>(bytes);
    case 1: return dileaveBytes_const<1>(bytes);
    case 2: return dileaveBytes_const<2>(bytes);
    case 3: return dileaveBytes_const<3>(bytes);
    case 4: return dileaveBytes_const<4>(bytes);
    case 5: return dileaveBytes_const<5>(bytes);
    case 6: return dileaveBytes_const<6>(bytes);
    case 7: return dileaveBytes_const<7>(bytes);
    case 8: return dileaveBytes_const<8>(bytes);
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

}  // namespace detail

constexpr u64 dileave_bytes(u64 bytes, usize count)
{
    return detail::dileaveBytes_jmp(bytes, count);
}

}  // namespace voxelio

#endif  // ILEAVE_HPP
