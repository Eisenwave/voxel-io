#ifndef VXIO_STRINGIFY_HPP
#define VXIO_STRINGIFY_HPP

#include "build.hpp"
#include "intlog.hpp"
#include "sstreamwrap.hpp"

#include <cmath>
#include <iosfwd>
#include <string>

// This dedicated header is necessary to avoid a circular dependency between util_string.hpp and assert.hpp.
// Assertions require this universal stringification to print compared values.

namespace voxelio {

namespace detail {

// simplified divCeil to avoid intdiv.hpp include
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint divCeil(Uint x, Uint y)
{
    return x / y + (x % y != 0);
}

inline void reverseString(std::string &str)
{
    size_t length = str.size();
    for (size_t i = 0; i < length / 2; i++) {
        std::swap(str[i], str[length - i - 1]);
    }
}

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

template <size_t BASE, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
std::string stringifyUInt(const Uint n) noexcept
{
    static_assert(BASE <= 16);

    constexpr const char *hexDigits = "0123456789abcdef";

    if (n < BASE) {
        return std::string{hexDigits[n]};
    }

    const size_t digitCount = voxelio::digitCount<BASE>(n);

    if constexpr (isPow2(BASE)) {
        constexpr size_t bitsPerDigit = log2floor(BASE);
        constexpr Uint digitMask = BASE - 1;

        std::string result(digitCount, 0);

        for (size_t i = 0, shift = (digitCount - 1) * bitsPerDigit; i < digitCount; ++i, shift -= bitsPerDigit) {
            Uint digit = (n >> shift) & digitMask;
            if constexpr (BASE == 2) {
                result[i] = '0' + digit;
            }
            else {
                result[i] = hexDigits[digit];
            }
        }

        return result;
    }
    else {
        std::string result;
        result.reserve(digitCount);

        // TODO consider assigning the characters directly instead of reversing the string

        for (Uint x = n; x != 0; x /= BASE) {
            result.push_back(hexDigits[x % BASE]);
        }

        reverseString(result);
        return result;
    }
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

/**
 * @brief Converts a floating point number to a string with precision of choice.
 * The resuling number will have at most <precision> fractional digits.
 * @tparam Float the floating point type
 * @param f the number
 * @param precision the precision
 * @return the stringified number
 */
template <size_t RADIX, typename Int, std::enable_if_t<(std::is_integral_v<Int> && RADIX > 1 && RADIX <= 16), int> = 0>
std::string stringifyInt(Int n) noexcept
{
    if constexpr (std::is_signed_v<Int>) {
        using Uint = std::make_unsigned_t<Int>;
        return n < 0 ? '-' + stringifyInt<RADIX, Uint>(static_cast<Uint>(-n))
                     : stringifyInt<RADIX, Uint>(static_cast<Uint>(n));
    }
    else {
        return detail::stringifyUInt<RADIX, Int>(n);
    }
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
 * @brief Stringifies a number in base 8 (octal).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyOct(T num) noexcept
{
    return stringifyInt<8>(num);
}

/**
 * @brief Stringifies a number in base 2 (binary).
 * @param num the number
 * @return the stringified number
 */
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string stringifyBin(T num) noexcept
{
    return stringifyInt<2>(num);
}

template <typename T>
std::string stringify(const T &t) noexcept
{
    if constexpr (std::is_same_v<bool, T>) {
        return t ? "true" : "false";
    }
    else if constexpr (std::is_integral_v<T>) {
        return stringifyDec(t);
    }
    else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(t);
    }
    else if constexpr (std::is_enum_v<T>) {
        return stringify(static_cast<std::underlying_type_t<T>>(t));
    }
    else if constexpr (std::is_same_v<std::string, T>) {
        return t;
    }
    else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string{t};
    }
    else if constexpr (std::is_same_v<const char *, const T>) {
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
