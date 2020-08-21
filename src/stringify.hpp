#ifndef VXIO_STRINGIFY_HPP
#define VXIO_STRINGIFY_HPP
/*
 * stringify.hpp
 * -----------
 * Provides various stringification functions. Most notably, there is a universal stringify() which can accept enums,
 * integers, floats, anything with a stream operator, pointers, c-strings, std::string, etc.
 * This is crucial for implementing stringification inside the ASSERT macros.
 */

#include "build.hpp"
#include "intlog.hpp"
#include "sstreamwrap.hpp"

#include <cmath>
#include <iosfwd>
#include <string>

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

template <size_t BASE, typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
std::string stringifyUInt(const Uint n) noexcept
{
    static_assert(BASE <= 16);

    constexpr size_t maxDigits = voxelio::digitCount<BASE>(std::numeric_limits<Uint>::max());
    constexpr const char *hexDigits = "0123456789abcdef";

    char result[maxDigits];
    Uint x = n;

    if constexpr (isPow2(BASE)) {
        constexpr size_t bitsPerDigit = log2floor(BASE);
        constexpr Uint digitMask = BASE - 1;

        const size_t digitCount = voxelio::digitCount<BASE>(n);

        size_t i = digitCount;
        do {
            result[--i] = hexDigits[x & digitMask];
            x >>= bitsPerDigit;
        } while (x != 0);

        return std::string(result, digitCount);
    }
    else {
        size_t i = maxDigits;
        do {
            result[--i] = hexDigits[x % BASE];
            x /= BASE;
        } while (x != 0);

        return std::string(result + i, maxDigits - i);
    }
}

}  // namespace detail

// NUMERIC TYPE STRINGIFICATION ========================================================================================

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

// UNIVERSAL STRINGIFICATION ===========================================================================================

template <typename T>
std::string stringify(const T &t) noexcept
{
    if constexpr (std::is_same_v<bool, T>) {
        return t ? "true" : "false";
    }
    else if constexpr (std::is_integral_v<T>) {
        return std::to_string(t);
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

// RATIONAL STRINGIFICATION ============================================================================================

/**
 * @brief Stringifies a fraction.
 * This function terminates once a division is possible without remainder or once the precision is reached.
 * @param num the numerator
 * @param den the denominator
 * @param precision the maximum fractional part precision
 * @return the stringified fraction
 */
std::string stringifyFraction(uint32_t num, uint32_t den, unsigned precision = 4) noexcept;

std::string stringifyFraction(uint64_t num, uint64_t den, unsigned precision = 4) noexcept;

/**
 * @brief Identical to fraction_to_string, but will right-pad the string with 0 until precision is reached.
 * @param num the numerator
 * @param den the denominator
 * @param precision the exact fractional part precision
 * @return the stringified fraction
 */
std::string stringifyFractionRpad(uint32_t num, uint32_t den, unsigned precision = 4) noexcept;

std::string stringifyFractionRpad(uint64_t num, uint64_t den, unsigned precision = 4) noexcept;

// SPECIAL PURPOSE STRINGIFICATION =====================================================================================

/**
 * @brief Creates a human-readable string from a file size given in bytes.
 * @param size the number of bytes
 * @param precision the maximum number of decimals to print
 * @tparam BASE either 1000 or 1024 depending on whether decimal units (KB) or digital units (KiB) should be used
 * @tparam SPACE true if a space should be inserted between the number and unit (true by default)
 * @return a human-readable string representing the given file size
 */
template <size_t BASE = 1024, bool SPACE = true, std::enable_if_t<BASE == 1000 || BASE == 1024, size_t> = BASE>
std::string stringifyFileSize(uint64_t size, unsigned precision = 0) noexcept
{
    constexpr const char FILE_SIZE_UNITS[8][3]{"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB"};

    uint64_t unit = logFloor<BASE>(size);

    std::string result = voxelio::stringifyFraction(size, powConst<BASE>(unit), precision);
    result.reserve(result.size() + 5);

    if constexpr (SPACE) {
        result.push_back(' ');
    }
    result.push_back(FILE_SIZE_UNITS[unit][0]);

    if (unit != 0) {
        if constexpr (BASE == 1024) {
            result.push_back('i');
        }
        result.push_back(FILE_SIZE_UNITS[unit][1]);
    }

    return result;
}

/**
 * @brief Creates a human-readable string from a time given in nanoseconds. Example:
 * stringifyTime(10'500, 1) =  10.5 us
 * @param nanos the nanoseconds to stringify
 * @param precision the maximum number of decimals to print
 * @tparam SPACE true if a space should be inserted between the number and unit (true by default)
 * @return a human-readable string representing the given number of nanoseconds
 */
template <bool SPACE = true>
std::string stringifyTime(uint64_t nanos, unsigned precision = 0) noexcept
{
    // clang-format off
    // TIME_FACTORS represents the factor to get to the next unit
    // the last factor is 10 (century -> millenium)
    constexpr size_t UNITS = 10;
    constexpr const char TIME_UNITS[UNITS][4]    {"ns", "us", "ms", "s", "min", "h", "d", "y", "dec", "cen"};
    constexpr const unsigned TIME_FACTORS[UNITS] {1000, 1000, 1000,  60,   60,  24,  365,  10,    10,    10};
    // clang-format on

    size_t unit = 0;
    uint64_t divisor = 1;
    for (; unit < 10; ++unit) {
        uint64_t nextDivisor = divisor * TIME_FACTORS[unit];
        if (nextDivisor >= nanos) {
            break;
        }
        divisor = nextDivisor;
    }

    std::string result = voxelio::stringifyFraction(nanos, divisor, precision);
    result.reserve(result.size() + 5);

    if constexpr (SPACE) {
        result.push_back(' ');
    }
    result.append(TIME_UNITS[unit]);

    return result;
}

}  // namespace voxelio

#endif  // VXIO_STRINGIFY_HPP
