#include "stringify.hpp"

#include "assert.hpp"

namespace voxelio {

template <bool RPAD, typename Int>
static std::string fraction_to_string_impl(Int num, const Int den, const unsigned precision)
{
    constexpr unsigned base = 10;

    if (den == 0) {
        return "inf";
    }
    std::string result = stringify(num / den);

    // integral part
    num %= den;
    if constexpr (not RPAD) {
        if (num == 0 || precision == 0) {
            return result;
        }
    }

    result.reserve(result.size() + precision + 1);

    // fractional part
    result += '.';
    if constexpr (RPAD) {
        for (size_t i = 0; i < precision; ++i) {
            num *= base;
            result += stringify(num / den);
            num %= den;
        }
    }
    else {
        for (size_t i = 0; i < precision && num != 0; ++i) {
            num *= base;
            result += stringify(num / den);
            num %= den;
        }
    }

    return result;
}

std::string stringifyFraction(uint32_t num, uint32_t den, unsigned precision) noexcept
{
    return fraction_to_string_impl<false>(num, den, precision);
}

std::string stringifyFraction(uint64_t num, uint64_t den, unsigned precision) noexcept
{
    return fraction_to_string_impl<false>(num, den, precision);
}

std::string stringifyFractionRpad(uint32_t num, uint32_t den, unsigned precision) noexcept
{
    return fraction_to_string_impl<true>(num, den, precision);
}

std::string stringifyFractionRpad(uint64_t num, uint64_t den, unsigned precision) noexcept
{
    return fraction_to_string_impl<true>(num, den, precision);
}

}  // namespace voxelio
