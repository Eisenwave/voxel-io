#ifndef VXIO_STRINGIFY_HPP
#define VXIO_STRINGIFY_HPP

#include "build.hpp"
#include "log2.hpp"
#include "sstreamwrap.hpp"

#include <iosfwd>
#include <string>

// This dedicated header is necessary to avoid a circular dependency between util_string.hpp and assert.hpp.
// Assertions require this universal stringification to print compared values.

namespace voxelio {

namespace detail {

/**
 * @brief Stringifies a type using a stringstream.
 */
template <typename T>
std::string stringifyUsingStream(const T &t) noexcept
{
    // Thanks to our implementation which hides construction and destruction of stringstream behind make/free functions,
    // we don't need to include <sstream>.
    //
    // This header (stringify.hpp) is being included by many, many other headers on way or the other.
    // Henceforth, optimizing its includes is very important to avoid including <iostream> in the whole project.

    std::stringstream *stream = detail::stringstream_make();
    *reinterpret_cast<std::ostream *>(stream) << t;
    std::string result = detail::stringstream_to_string(stream);
    detail::stringstream_free(stream);
    return result;
}

template <size_t RADIX, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
std::string stringifyUIntBasePow2(const Uint n) noexcept
{
    static_assert(RADIX <= 16);

    static constexpr size_t bitsPerDigit = log2floor(RADIX);
    static constexpr Uint radixMask = RADIX - 1;
    static constexpr const char *hexDigits = "0123456789abcdef";

    if (n < RADIX) {
        return std::string{hexDigits[n]};
    }

    const auto bitCount = log2floor(n) + 1;
    const auto digitCount = 1 + ((bitCount - 1) / bitsPerDigit);  // integer division with ceil rounding

    std::string result(digitCount, '0');

    for (size_t i = 0, shift = (digitCount - 1) * bitsPerDigit; i < digitCount; ++i, shift -= bitsPerDigit) {
        result[i] = hexDigits[(n >> shift) & radixMask];
    }

    return result;
}

}  // namespace detail

/**
 * @brief Stringifies a fraction.
 * This function terminates once a division is possible without remainder or once the precision is reached.
 * @param num the numerator
 * @param den the denominator
 * @param precision the maximum fractional part precision
 * @return the stringified fraction
 */
std::string stringifyFraction(unsigned num, unsigned den, unsigned precision = 4) noexcept;

/**
 * @brief Identical to fraction_to_string, but will right-pad the string with 0 until precision is reached.
 * @param num the numerator
 * @param den the denominator
 * @param precision the exact fractional part precision
 * @return the stringified fraction
 */
std::string stringifyFractionRpad(unsigned num, unsigned den, unsigned precision = 4) noexcept;

/**
 * @brief Converts a floating point number to a string with precision of choice.
 * The resuling number will have at most <precision> fractional digits.
 * @tparam Float the floating point type
 * @param f the number
 * @param precision the precision
 * @return the stringified number
 */
template <typename Float, std::enable_if_t<std::is_floating_point_v<Float>, int> = 0>
std::string stringifyFloat(Float f, size_t precision) noexcept
{
    std::stringstream *stream = detail::stringstream_make();
    detail::stringstream_precision(stream, static_cast<std::streamsize>(precision));
    *reinterpret_cast<std::ostream *>(stream) << f;
    std::string result = detail::stringstream_to_string(stream);
    detail::stringstream_free(stream);
    return result;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyBin(T num) noexcept;

/**
 * @brief Converts a floating point number to a string with precision of choice.
 * The resuling number will have at most <precision> fractional digits.
 * @tparam Float the floating point type
 * @param f the number
 * @param precision the precision
 * @return the stringified number
 */
template <size_t RADIX, typename Int, std::enable_if_t<(std::is_integral_v<Int> && RADIX > 1), int> = 0>
std::string stringifyInt(Int n) noexcept
{
    if constexpr (RADIX == 2) {
        return stringifyBin(n);
    }
    else if constexpr (RADIX == 10) {
        return std::to_string(n);
    }
    else if constexpr (const bool is_pow_2 = (RADIX & (RADIX - 1)) == 0; is_pow_2) {
        if constexpr (std::is_signed_v<Int>) {
            using Uint = std::make_unsigned_t<Int>;
            return n < 0 ? '-' + stringifyInt<RADIX, Uint>(static_cast<Uint>(-n))
                         : stringifyInt<RADIX, Uint>(static_cast<Uint>(n));
        }
        else {
            return detail::stringifyUIntBasePow2<RADIX, Int>(n);
        }
    }
    else {
        return nullptr;
    }
}

/**
 * @brief Stringifies a number in base 2 (binary).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int>>
std::string stringifyBin(T num) noexcept
{
    constexpr size_t bufferSize = std::numeric_limits<T>::digits;
    constexpr size_t maxIndex = bufferSize - 1 - std::is_signed_v<T>;

    if (num < 2) {
        return std::string{static_cast<char>('0' + num)};
    }

    char result[bufferSize];
    size_t maxBit VXIO_IF_DEBUG(= SIZE_MAX);
    bool negative = false;

    if constexpr (std::is_signed_v<T>) {
        negative = num < 0;
        if (negative) {
            result[0] = '-';
            num = -num;
        }
    }

    for (size_t bit = 0; num != 0; num >>= 1, bit++) {
        size_t index = maxIndex - bit;
        result[index] = '0' + (num & 1);
        if (num & 1) {
            maxBit = bit;
        }
    }

    char *ptr = result + maxIndex - maxBit - negative;
    return {ptr, maxBit + 1};
}

/**
 * @brief Stringifies a number in base 10 (decimal).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyDec(T num) noexcept
{
    return stringifyInt<10>(num);
}

/**
 * @brief Stringifies a number in base 16 (hexadecimal).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyHex(T num) noexcept
{
    return stringifyInt<16>(num);
}

/**
 * @brief Stringifies a number in base 8 (octal).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyOct(T num) noexcept
{
    return stringifyInt<8>(num);
}

template <typename T>
std::string stringify(const T &t) noexcept
{
    if constexpr (std::is_same_v<bool, T>) {
        return t ? "true" : "false";
    }
    else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(t);
    }
    else if constexpr (std::is_enum_v<T>) {
        return stringify(static_cast<std::underlying_type_t<T>>(t));
    }
    else if constexpr (std::is_same_v<std::string, T>) {
        return t;
    }
    else if constexpr (std::is_same_v<const char *, T>) {
        return std::string{t};
    }
    else if constexpr (std::is_pointer_v<T>) {
        return stringifyHex(reinterpret_cast<uintptr_t>(t));
    }
    else {
        return detail::stringifyUsingStream<T>(t);
    }
}

}  // namespace voxelio

#endif  // VXIO_STRINGIFY_HPP
