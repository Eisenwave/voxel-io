#include "voxelio/stringify.hpp"

#include "voxelio/assert.hpp"

namespace voxelio {

namespace {

template <bool RPAD, typename Int>
std::string fractionToString_impl(Int num, const Int den, const unsigned precision)
{
    static constexpr unsigned base = 10;

    if (den == 0) {
        return "inf";
    }
    std::string result = stringify(num / den);

    // integral part
    num %= den;
    if (precision == 0 || (not RPAD && num == 0)) {
        return result;
    }

    result.reserve(result.size() + precision + 1);

    // fractional part
    result.push_back('.');

    for (size_t i = 0; i < precision && (RPAD || num != 0); ++i) {
        num *= base;
        char digit = static_cast<char>(num / den) + '0';
        result.push_back(digit);
        num %= den;
    }

    return result;
}

template <usize BASE = 1024, bool SPACE = true, std::enable_if_t<BASE == 1000 || BASE == 1024, usize> = BASE>
std::string stringifyFileSize_impl(uint64_t size, unsigned precision = 0, char separator = ' ') noexcept
{
    static constexpr const char FILE_SIZE_UNITS[8][3]{"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB"};

    uint64_t unit = logFloor<BASE>(size);

    std::string result = voxelio::stringifyFraction(size, powConst<BASE>(unit), precision);
    result.reserve(result.size() + 5);

    if (separator != 0) {
        result.push_back(separator);
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

}  // namespace

std::string stringifyFraction(uint32_t num, uint32_t den, unsigned precision) noexcept
{
    return fractionToString_impl<false>(num, den, precision);
}

std::string stringifyFraction(uint64_t num, uint64_t den, unsigned precision) noexcept
{
    return fractionToString_impl<false>(num, den, precision);
}

std::string stringifyFractionRpad(uint32_t num, uint32_t den, unsigned precision) noexcept
{
    return fractionToString_impl<true>(num, den, precision);
}

std::string stringifyFractionRpad(uint64_t num, uint64_t den, unsigned precision) noexcept
{
    return fractionToString_impl<true>(num, den, precision);
}

std::string stringifyLargeInt(uint64_t num, char separator) noexcept
{
    std::string result = stringifyDec(num);
    for (usize n = result.length(); n > 3;) {
        result.insert(n -= 3, {separator});
    }
    return result;
}

std::string stringifyFileSize1000(uint64_t num, unsigned precision, char separator) noexcept
{
    return stringifyFileSize_impl<1000>(num, precision, separator);
}

std::string stringifyFileSize1024(uint64_t num, unsigned precision, char separator) noexcept
{
    return stringifyFileSize_impl<1024>(num, precision, separator);
}

std::string stringifyTime(uint64_t nanos, unsigned precision, char separator) noexcept
{
    // clang-format off
    // TIME_FACTORS represents the factor to get to the next unit
    // the last factor is 10 (century -> millenium)
    static constexpr usize UNITS = 10;
    static constexpr const char TIME_UNITS[UNITS][4]    {"ns", "us", "ms", "s", "min", "h", "d", "y", "dec", "cen"};
    static constexpr const unsigned TIME_FACTORS[UNITS] {1000, 1000, 1000,  60,   60,  24,  365,  10,    10,    10};
    // clang-format on

    usize unit = 0;
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

    if (separator != 0) {
        result.push_back(separator);
    }
    result.append(TIME_UNITS[unit]);

    return result;
}

}  // namespace voxelio
