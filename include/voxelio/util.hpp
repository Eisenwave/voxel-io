#ifndef VXIO_UTIL_HPP
#define VXIO_UTIL_HPP
/*
 * util.hpp
 * --------
 * Provides fundamental utilities such as isConstantEvaluated(), a safe downcast, etc.
 */

#include "assert.hpp"
#include "builtin.hpp"
#include "langext.hpp"
#include "primitives.hpp"

#include <cstring>
#include <type_traits>

namespace voxelio {

#if 0
namespace detail {


template <typename T>
struct Swappability {
    template <typename T2 = T>
    static auto swappableByMember(...)
    -> std::false_type;

    template <typename T2 = T>
    static auto swappableByMember(int)
    -> decltype(std::declval<T2&>().swap(std::declval<T2&>()), std::true_type{});

    template <typename T2 = T>
    static auto nothrowSwappablyByMember(...)
    -> std::false_type;

    template <typename T2 = T>
    static auto nothrowSwappablyByMember(int)
    -> std::integral_constant<bool, noexcept(std::declval<T2&>().swap(std::declval<T2&>()))>;


    static constexpr bool byMember = decltype(swappableByMember(0))::value;
    static constexpr bool nothrowByMember = decltype(nothrowSwappablyByMember(0))::value;
};

}

template <typename T>
constexpr bool isSwappableByMemberFunction = detail::Swappability<T>::byMember;

template <typename T>
constexpr bool isNothrowSwappableByMemberFunction = detail::Swappability<T>::nothrowByMember;
#endif

/**
 * @brief constexpr swap for trivial types.
 * This is necessary because std::swap is not constexpr pre C++20.
 *
 * It is constrained to trivial types so that we don't have to worry about noexcept correctness.
 */
template <typename T>
constexpr std::enable_if_t<std::is_trivial_v<T>, void> trivialSwap(T &a, T &b) noexcept
{
    T tmp = a;
    a = b;
    b = tmp;
}

/**
 * @brief performs downcast of one reference to another.
 * On debug builds, this will terminate if the cast fails.
 */
template <typename DERIVED, typename BASE>
constexpr std::enable_if_t<std::is_reference_v<DERIVED>, DERIVED> downcast(BASE &src) noexcept
{
    return VXIO_IF_DEBUG_ELSE(dynamic_cast<DERIVED>(src), static_cast<DERIVED>(src));
}

/**
 * @brief Performs a downcast of one pointer to another.
 * On debug builds, this will terminate if the cast fails.
 * Downcasting a nullptr is always allowed.
 */
template <typename DERIVED, typename BASE>
constexpr std::enable_if_t<std::is_pointer_v<DERIVED>, DERIVED> downcast(BASE *src) noexcept
{
    if constexpr (build::DEBUG) {
        DERIVED result = dynamic_cast<DERIVED>(src);
        VXIO_DEBUG_ASSERT_CONSEQUENCE(src != nullptr, result != nullptr);
        return result;
    }
    else {
        return static_cast<DERIVED>(src);
    }
}

/**
 * @brief Finds the first mismatch between two arrays.
 * If no mismatch was found, size is returned.
 * @param arr0 the first array
 * @param arr1 the second array
 * @param size the common size of both arrays
 * @return the index of the first mismatch
 */
template <typename T>
constexpr usize mismatchIndex(const T arr0[], const T arr1[], usize size) noexcept
{
    usize i = 0;
    for (; i < size; ++i) {
        if (arr0[i] != arr1[i]) {
            break;
        }
    }
    return i;
}

template <typename T>
constexpr std::underlying_type_t<T> toUnderlying(T t) noexcept
{
    return static_cast<std::underlying_type_t<T>>(t);
}

template <typename T>
constexpr std::make_unsigned_t<T> toUnsigned(T t) noexcept
{
    return static_cast<std::make_unsigned_t<T>>(t);
}

template <typename T>
constexpr std::make_signed_t<T> toSigned(T t) noexcept
{
    return static_cast<std::make_signed_t<T>>(t);
}

}  // namespace voxelio

#endif  // UTIL_HPP
