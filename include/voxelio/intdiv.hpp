#ifndef VXIO_INTDIV_HPP
#define VXIO_INTDIV_HPP
/*
 * intdiv.hpp
 * -----------
 * Implements ceil, floor, outwards rounding modes for integer division.
 * - cloor is often necessary to get a consistent space downscaling.
 * - ceil is often necessary to get the size of a containing array of data which is not aligned to the container size
 */

#include <type_traits>

namespace voxelio {

/**
 * @brief Similar to std::common_type_t<A, B>, but if A or B are signed, the result will also be signed.
 *
 * This differs from the regular type promotion rules, where signed types are promoted to unsigned types.
 */
template <typename A, typename B, std::enable_if_t<std::is_integral_v<A> && std::is_integral_v<B>, int> = 0>
using commonSignedType = std::conditional_t<std::is_unsigned_v<A> && std::is_unsigned_v<B>,
                                            std::common_type_t<A, B>,
                                            std::common_type_t<std::make_signed_t<A>, std::make_signed_t<B>>>;

static_assert(std::is_same_v<unsigned, commonSignedType<unsigned, unsigned>>);
static_assert(std::is_same_v<int, commonSignedType<unsigned, int>>);
static_assert(std::is_same_v<int, commonSignedType<int, unsigned>>);
static_assert(std::is_same_v<int, commonSignedType<int, int>>);

/**
 * Performs a division but rounds towards positive infinity.
 *
 * @param x the dividend
 * @param y the divisor
 * @return ceil(x / y)
 */
template <typename Dividend,
          typename Divisor,
          std::enable_if_t<std::is_integral_v<Dividend> && std::is_integral_v<Divisor>, int> = 0>
constexpr commonSignedType<Dividend, Divisor> divCeil(Dividend x, Divisor y)
{
    if constexpr (std::is_unsigned_v<Dividend> && std::is_unsigned_v<Divisor>) {
        // quotient is always positive
        return x / y + (x % y != 0);  // uint / uint
    }
    else if constexpr (std::is_signed_v<Dividend> && std::is_unsigned_v<Divisor>) {
        auto sy = static_cast<std::make_signed_t<Divisor>>(y);
        bool quotientPositive = x >= 0;
        return x / sy + (x % sy != 0 && quotientPositive);  // int / uint
    }
    else if constexpr (std::is_unsigned_v<Dividend> && std::is_signed_v<Divisor>) {
        auto sx = static_cast<std::make_signed_t<Dividend>>(x);
        bool quotientPositive = y >= 0;
        return sx / y + (sx % y != 0 && quotientPositive);  // uint / int
    }
    else {
        bool quotientPositive = (y >= 0) == (x >= 0);
        return x / y + (x % y != 0 && quotientPositive);  // int / int
    }
}

/**
 * Performs a division but rounds towards negative infinity.
 * For positive numbers, this is equivalent to regular division.
 * For all numbers, this is equivalent to a floating point division and then a floor().
 * Negative numbers will be decremented before division, leading to this type of rounding.
 *
 * Examples:
 * floor(-1/2) = floor(-0.5) = -1
 * floor(-2/2) = floor(-1) = -1
 *
 * This function imitates such behavior but without the use of any floating point arithmetic.
 *
 * @param x the dividend
 * @param y the divisor
 * @return floor(x / y)
 */
template <typename Dividend,
          typename Divisor,
          std::enable_if_t<std::is_integral_v<Dividend> && std::is_integral_v<Divisor>, int> = 0>
constexpr commonSignedType<Dividend, Divisor> divFloor(Dividend x, Divisor y)
{
    if constexpr (std::is_unsigned_v<Dividend> && std::is_unsigned_v<Divisor>) {
        // quotient is never negative
        return x / y;  // uint / uint
    }
    else if constexpr (std::is_signed_v<Dividend> && std::is_unsigned_v<Divisor>) {
        auto sy = static_cast<std::make_signed_t<Divisor>>(y);
        bool quotientNegative = x < 0;
        return x / sy - (x % sy != 0 && quotientNegative);  // int / uint
    }
    else if constexpr (std::is_unsigned_v<Dividend> && std::is_signed_v<Divisor>) {
        auto sx = static_cast<std::make_signed_t<Dividend>>(x);
        bool quotientNegative = y < 0;
        return sx / y - (sx % y != 0 && quotientNegative);  // uint / int
    }
    else {
        bool quotientNegative = (y < 0) != (x < 0);
        return x / y - (x % y != 0 && quotientNegative);  // int / int
    }
}

template <typename Dividend,
          typename Divisor,
          std::enable_if_t<std::is_integral_v<Dividend> && std::is_integral_v<Divisor>, int> = 0>
constexpr commonSignedType<Dividend, Divisor> divUp(Dividend x, Divisor y)
{
    constexpr auto sgn = [](commonSignedType<Dividend, Divisor> n) -> signed char {
        return (n > 0) - (n < 0);
    };

    if constexpr (std::is_unsigned_v<Dividend> && std::is_unsigned_v<Divisor>) {
        // sgn is always 1
        return x / y + (x % y != 0);  // uint / uint
    }
    else if constexpr (std::is_signed_v<Dividend> && std::is_unsigned_v<Divisor>) {
        auto sy = static_cast<std::make_signed_t<Divisor>>(y);
        signed char quotientSgn = sgn(x);
        return x / sy + (x % sy != 0) * quotientSgn;  // int / uint
    }
    else if constexpr (std::is_unsigned_v<Dividend> && std::is_signed_v<Divisor>) {
        auto sx = static_cast<std::make_signed_t<Dividend>>(x);
        signed char quotientSgn = sgn(y);
        return sx / y + (sx % y != 0) * quotientSgn;  // uint / int
    }
    else {
        signed char quotientSgn = sgn(x) * sgn(y);
        return x / y + (x % y != 0) * quotientSgn;  // int / int
    }
}

}  // namespace voxelio

#endif  // INTDIV_HPP
