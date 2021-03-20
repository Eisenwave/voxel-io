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

#include "bits.hpp"
#include "build.hpp"
#include "builtin.hpp"
#include "primitives.hpp"
#include "util.hpp"

#include <cstring>

namespace voxelio {

using build::Endian;

// IMPLEMENTATION ======================================================================================================

namespace detail {

template <Endian ENDIAN, typename Int>
constexpr void encode_reverse(Int integer, u8 out[sizeof(Int)])
{
    if constexpr (ENDIAN != Endian::NATIVE) {
        integer = reverseBytes(integer);
    }
    std::memcpy(out, &integer, sizeof(Int));
}

template <Endian ENDIAN, typename Int>
constexpr Int decode_reverse(const u8 buffer[sizeof(Int)])
{
    Int result = 0;
    std::memcpy(&result, buffer, sizeof(Int));
    if constexpr (ENDIAN != Endian::NATIVE) {
        result = reverseBytes(result);
    }
    return result;
}

template <Endian ENDIAN, typename Int>
constexpr void encode_naive(Int integer, u8 out[sizeof(Int)])
{
    for (usize i = 0; i < sizeof(Int); ++i) {
        usize shift = 0;
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
    for (usize i = 0; i < sizeof(Int); ++i) {
        usize shift = 0;
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

template <Endian ENDIAN, typename Int>
constexpr Int decode(const u8 buffer[sizeof(Int)])
{
    return detail::decode_reverse<ENDIAN, Int>(buffer);
}

template <Endian ENDIAN, typename Int>
constexpr void encode(Int integer, u8 out[sizeof(Int)])
{
    return detail::encode_reverse<ENDIAN, Int>(integer, out);
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

/**
 * @brief Convenience function that forwards to decode<Endian::LITTLE>.
 */
template <typename Int>
constexpr Int decodeLittle(const u8 buffer[sizeof(Int)])
{
    return decode<Endian::LITTLE, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to decode<Endian::BIG>.
 */
template <typename Int>
constexpr Int decodeBig(const u8 buffer[sizeof(Int)])
{
    return decode<Endian::BIG, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to decode<Endian::NATIVE>.
 */
template <typename Int>
constexpr Int decodeNative(const u8 buffer[sizeof(Int)])
{
    return decode<Endian::NATIVE, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to encode<Endian::LITTLE>.
 */
template <typename Int>
constexpr void encodeLittle(Int integer, u8 out[])
{
    encode<Endian::LITTLE>(integer, out);
}

/**
 * @brief Convenience function that forwards to encode<Endian::BIG>.
 */
template <typename Int>
constexpr void encodeBig(Int integer, u8 out[])
{
    encode<Endian::BIG>(integer, out);
}

/**
 * @brief Convenience function that forwards to encode<Endian::NATIVE>.
 */
template <typename Int>
constexpr void encodeNative(Int integer, u8 out[])
{
    encode<Endian::NATIVE>(integer, out);
}

}  // namespace voxelio

#endif  // ENDIAN_HPP
