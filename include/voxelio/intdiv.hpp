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
constexpr commonSignedType<Dividend, Divisor> divCeil(Dividend x, Divisor y) noexcept
{
    const bool quotientPositive = (x >= 0) == (y >= 0);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy + (cx % cy != 0 && quotientPositive);
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
constexpr commonSignedType<Dividend, Divisor> divFloor(Dividend x, Divisor y) noexcept
{
    const bool quotientNegative = (x >= 0) != (y >= 0);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy - (cx % cy != 0 && quotientNegative);
}

namespace detail {

template <typename T>
constexpr signed char divUp_sgn(T n)
{
    if constexpr (std::is_unsigned_v<T>) {
        return 1;
    }
    else {
        return (n > T{0}) - (n < T{0});
    }
};

}  // namespace detail

template <typename Dividend,
          typename Divisor,
          std::enable_if_t<std::is_integral_v<Dividend> && std::is_integral_v<Divisor>, int> = 0>
constexpr commonSignedType<Dividend, Divisor> divUp(Dividend x, Divisor y) noexcept
{
    signed char quotientSgn = detail::divUp_sgn(x) * detail::divUp_sgn(y);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy + (cx % cy != 0) * quotientSgn;
}

template <typename Dividend,
          typename Divisor,
          std::enable_if_t<std::is_integral_v<Dividend> && std::is_integral_v<Divisor>, int> = 0>
constexpr commonSignedType<Dividend, Divisor> divTrunc(Dividend x, Divisor y) noexcept
{
    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy;
}

}  // namespace voxelio

#endif  // INTDIV_HPP
