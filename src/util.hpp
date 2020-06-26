#ifndef VXIO_UTIL_HPP
#define VXIO_UTIL_HPP

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

}  // namespace voxelio

#endif  // UTIL_HPP
