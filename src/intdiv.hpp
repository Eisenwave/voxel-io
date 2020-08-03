#ifndef VXIO_INTDIV_HPP
#define VXIO_INTDIV_HPP

#include <type_traits>

namespace voxelio {

/**
 * Performs a division but rounds towards positive infinity.
 *
 * @param x the dividend
 * @param y the divisor
 * @return ceil(x / y)
 */
template <typename Dividend,
          typename Divisor,
          std::enable_if_t<(std::is_integral_v<Dividend> && std::is_integral_v<Divisor>), int> = 0>
constexpr auto divCeil(Dividend x, Divisor y)
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
          std::enable_if_t<(std::is_integral_v<Dividend> && std::is_integral_v<Divisor>), int> = 0>
constexpr auto divFloor(Dividend x, Divisor y)
{
    if constexpr (std::is_unsigned_v<Dividend> && std::is_unsigned_v<Divisor>) {
        // quotient is never negative
        return x / y;
    }
    else if constexpr (std::is_signed_v<Dividend> && std::is_unsigned_v<Divisor>) {
        auto sy = static_cast<std::make_signed_t<Divisor>>(y);
        bool quotientNegative = x < 0;
        return x / sy - (x % sy != 0 && quotientNegative);
    }
    else if constexpr (std::is_unsigned_v<Dividend> && std::is_signed_v<Divisor>) {
        auto sx = static_cast<std::make_signed_t<Dividend>>(x);
        bool quotientNegative = y < 0;
        return sx / y - (sx % y != 0 && quotientNegative);
    }
    else {
        bool quotientNegative = (y < 0) != (x < 0);
        return x / y - (x % y != 0 && quotientNegative);
    }
}

// uint / uint
static_assert(divCeil(0u, 2u) == 0);
static_assert(divCeil(2u, 1u) == 2);
static_assert(divCeil(3u, 2u) == 2);

// int / uint
static_assert(divCeil(0, 2u) == 0);
static_assert(divCeil(2, 1u) == 2);
static_assert(divCeil(-3, 2u) == -1);
static_assert(divCeil(3, 2u) == 2);

// uint / int
static_assert(divCeil(0u, 2) == 0);
static_assert(divCeil(2u, 1) == 2);
static_assert(divCeil(3u, -2) == -1);
static_assert(divCeil(3u, 2) == 2);

// int / int
static_assert(divCeil(0, 2) == 0);
static_assert(divCeil(2, 1) == 2);
static_assert(divCeil(-3, -2) == 2);
static_assert(divCeil(3, 2) == 2);
static_assert(divCeil(-3, 2) == -1);
static_assert(divCeil(3, -2) == -1);

// uint / uint
static_assert(divFloor(0u, 2u) == 0);
static_assert(divFloor(3u, 2u) == 1);

// int / uint
static_assert(divFloor(0, 2u) == 0);
static_assert(divFloor(-2, 1u) == -2);
static_assert(divFloor(-3, 2u) == -2);
static_assert(divFloor(3, 2u) == 1);

// uint / int
static_assert(divFloor(0u, 2) == 0);
static_assert(divFloor(2u, 1) == 2);
static_assert(divFloor(3u, -2) == -2);
static_assert(divFloor(3u, 2) == 1);

// int / int
static_assert(divFloor(0, 2) == 0);
static_assert(divFloor(-2, 1) == -2);
static_assert(divFloor(-3, -2) == 1);
static_assert(divFloor(3, 2) == 1);
static_assert(divFloor(-3, 2) == -2);
static_assert(divFloor(3, -2) == -2);

}  // namespace voxelio

#endif  // INTDIV_HPP
