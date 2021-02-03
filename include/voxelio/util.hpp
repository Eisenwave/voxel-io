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

/**
 * @brief Performs a cast between types where the raw bits of one type are reinterpreted as that of another.
 * This allows reinterpretation of e.g. an int as a float without undefined behavior.
 * @param from the source type
 * @return the casted result
 */
template <typename To, typename From>
inline To bitcast(const From &from)
{
    static_assert(sizeof(To) == sizeof(From));
    To result;
    std::memcpy(&result, &from, sizeof(To));
    return result;
}

/**
 * @brief performs downcast of one reference to another.
 * On debug builds, this will terminate if the cast fails.
 */
template <typename DERIVED, typename BASE, std::enable_if_t<std::is_reference_v<DERIVED>, int> = 0>
constexpr DERIVED downcast(BASE &src) noexcept
{
    return VXIO_IF_DEBUG_ELSE(dynamic_cast<DERIVED>(src), static_cast<DERIVED>(src));
}

/**
 * @brief Performs a downcast of one pointer to another.
 * On debug builds, this will terminate if the cast fails.
 * Downcasting a nullptr is always allowed.
 */
template <typename DERIVED, typename BASE, std::enable_if_t<std::is_pointer_v<DERIVED>, int> = 0>
constexpr DERIVED downcast(BASE *src) noexcept
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
constexpr usize compareArrays(const T arr0[], const T arr1[], usize size)
{
    for (usize i = 0; i < size; ++i) {
        if (arr0[i] != arr1[i]) {
            return i;
        }
    }
    return size;
}

/**
 * @brief Reverses an array.
 * @param arr the array to reverse
 * @param size the size of the array
 */
template <typename T>
constexpr void reverseArray(T arr[], usize size)
{
    for (size_t i = 0; i < size / 2; ++i) {
        std::swap(arr[i], arr[size - i - 1]);
    }
}

}  // namespace voxelio

#endif  // UTIL_HPP
