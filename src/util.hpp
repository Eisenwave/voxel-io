#ifndef VXIO_UTIL_HPP
#define VXIO_UTIL_HPP

#include "builtin.hpp"
#include "primitives.hpp"

#include <cstring>
#include <type_traits>

namespace voxelio {

/**
 * @brief Returns true if the function call occurs within a constant-evaluated context.
 * It may also return true if such detection is not possible.
 *
 * This function is functionally identical to std::is_constant_evaluated(), but might be working pre C++20 too, thanks
 * to voxelio::builtin::isConstantEvaluated() if it is available.
 *
 * @return true if the function call occurs within a constant-evaluated context
 */
constexpr bool isConstantEvaluated()
{
#ifdef VXIO_HAS_BUILTIN_IS_CONSTANT_EVALUATED
    return builtin::isConstantEvaluated();
#elif defined(VXIO_CPP20_LEAST)
    return std::is_constant_evaluated();
#else
    return true;
#endif
}

template <typename To, typename From>
inline To memCast(const From &from)
{
    static_assert(sizeof(To) == sizeof(From));
    To result;
    std::memcpy(&result, &from, sizeof(To));
    return result;
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
