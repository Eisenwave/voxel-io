#ifndef VXIO_ENDIAN_HPP
#define VXIO_ENDIAN_HPP
/*
 * endian.hpp
 * -----------
 * Provides portable conversions between integers and byte arrays.
 * The encode() and decode() methods are the most notable functions in this header.
 * There are also convenience functions encodeBig(), decodeLittle(), ...
 *
 * Whenever possible, the conversion doesn't take place using bitwise operations but by using std::memcpy and reversing
 * the byte order using builtin::byteSwap.
 */

#include "builtin.hpp"
#include "types.hpp"
#include "util.hpp"

#include <cstring>

namespace voxelio {

// PORTABLE BYTE SWAP IMPLEMENTATION ===================================================================================

#ifdef VXIO_HAS_BUILTIN_BSWAP
#define VXIO_HAS_ACCELERATED_REVERSE_BYTES
/**
 * @brief Reverses the bytes of any integer.
 * This implementation uses the __builtin_bswapXX() pseudo-function.
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
        return builtin::byteSwap(integer);
    }
}
#else
/**
 * @brief Reverses the bytes of any integer.
 * This implementation does not use any builtin function.
 * @param integer the integer
 */
template <typename Int>
constexpr Int reverseBytes(Int integer)
{
    // Compiler Explorer experiments have shown that we can't use encode/decode with different endians, as gcc
    // loses the ability to recognize the byte swap with bswap automatically.
    // Even for this implementation, gcc only recognizes it up to uint32_t, clang recognizes it for 64 bits too.
    // This is why the builtin variant is quite useful.
    u8 octets[sizeof(Int)]{};
    for (size_t i = 0; i < sizeof(Int); ++i) {
        octets[i] = static_cast<u8>(integer >> (i * 8));
    }
    Int result = 0;
    for (size_t i = 0; i < sizeof(Int); ++i) {
        size_t shift = i * 8;
        result |= Int{octets[sizeof(Int) - i - 1]} << shift;
    }
    return result;
}
#endif

static_assert(reverseBytes(0x11223344) == 0x44332211);

// IMPLEMENTATION ======================================================================================================

namespace detail {
// The builtin version will only be chosen if VXIO_HAS_ACCELERATED_REVERSE_BYTES is defined.
// Both are defined so that they can be tested individually.

#ifdef VXIO_HAS_KNOWN_ENDIAN

template <Endian ENDIAN, typename Int>
void encode_builtin(Int integer, u8 out[sizeof(Int)])
{
    if constexpr (build::NATIVE_ENDIAN != ENDIAN) {
        integer = reverseBytes(integer);
    }
    std::memcpy(out, &integer, sizeof(Int));
}

template <Endian ENDIAN, typename Int>
Int decode_builtin(const u8 buffer[sizeof(Int)])
{
    Int result = 0;
    std::memcpy(&result, buffer, sizeof(Int));
    if constexpr (build::NATIVE_ENDIAN != ENDIAN) {
        result = reverseBytes(result);
    }
    return result;
}

#endif  // VXIO_HAS_KNOWN_ENDIAN

template <Endian ENDIAN, typename Int>
constexpr void encode_naive(Int integer, u8 out[sizeof(Int)])
{
    for (size_t i = 0; i < sizeof(Int); ++i) {
        size_t shift = 0;
        if constexpr (ENDIAN == Endian::LITTLE) {
            shift = (i * 8);
        }
        else {
            shift = (sizeof(Int) - i - 1) * 8;
        }
        out[i] = integer >> shift;
    }
}

template <Endian ENDIAN, typename Int>
constexpr Int decode_naive(const u8 buffer[sizeof(Int)])
{
    Int result = 0;
    for (size_t i = 0; i < sizeof(Int); ++i) {
        size_t shift = 0;
        if constexpr (ENDIAN == Endian::LITTLE) {
            shift = i * 8;
        }
        else {
            shift = (sizeof(Int) - i - 1) * 8;
        }
        result += buffer[i] << shift;
    }
    return result;
}

}  // namespace detail

#if defined(VXIO_HAS_KNOWN_ENDIAN) && defined(VXIO_HAS_ACCELERATED_REVERSE_BYTES)
#define VXIO_HAS_ACCELERATED_ENDIAN
#endif

template <Endian ENDIAN, typename Int>
constexpr Int decode(const u8 buffer[sizeof(Int)])
{
#ifdef VXIO_HAS_ACCELERATED_ENDIAN
    if (isConstantEvaluated()) {
        return detail::decode_naive<ENDIAN, Int>(buffer);
    }
    else {
        return detail::decode_builtin<ENDIAN, Int>(buffer);
    }
#else
    return detail::decode_naive<ENDIAN, Int>(buffer);
#endif
}

template <Endian ENDIAN, typename Int>
constexpr void encode(Int integer, u8 out[sizeof(Int)])
{
#ifdef VXIO_HAS_ACCELERATED_ENDIAN
    if (isConstantEvaluated()) {
        detail::encode_naive<ENDIAN, Int>(integer, out);
    }
    else {
        detail::encode_builtin<ENDIAN, Int>(integer, out);
    }
#else
    detail::encode_naive<ENDIAN, Int>(out);
#endif
}

// SPECIALIZATION ======================================================================================================

template <>
constexpr u8 decode<Endian::LITTLE, u8>(const u8 buffer[1])
{
    return buffer[0];
}

template <>
constexpr u8 decode<Endian::BIG, u8>(const u8 buffer[1])
{
    return buffer[0];
}

template <>
constexpr char decode<Endian::LITTLE, char>(const u8 buffer[1])
{
    return static_cast<char>(buffer[0]);
}

template <>
constexpr char decode<Endian::BIG, char>(const u8 buffer[1])
{
    return static_cast<char>(buffer[0]);
}

template <>
constexpr i8 decode<Endian::LITTLE, i8>(const u8 buffer[1])
{
    return static_cast<i8>(buffer[0]);
}

template <>
constexpr i8 decode<Endian::BIG, i8>(const u8 buffer[1])
{
    return static_cast<i8>(buffer[0]);
}

// CONVENIENCE FUNCTIONS ===============================================================================================

template <typename Int>
constexpr Int decodeLittle(const u8 buffer[sizeof(Int)])
{
    return decode<Endian::LITTLE, Int>(buffer);
}

template <typename Int>
constexpr Int decodeBig(const u8 buffer[sizeof(Int)])
{
    return decode<Endian::BIG, Int>(buffer);
}

template <typename Int>
constexpr void encodeLittle(Int integer, u8 out[])
{
    encode<Endian::LITTLE>(integer, out);
}

template <typename Int>
constexpr void encodeBig(Int integer, u8 out[])
{
    encode<Endian::BIG>(integer, out);
}

}  // namespace voxelio

#endif  // ENDIAN_HPP
