#ifndef VXIO_LANGEXT_HPP
#define VXIO_LANGEXT_HPP

#include "builtin.hpp"

#ifdef VXIO_CPP20_LEAST
#include <type_traits>
#endif

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

#ifdef VXIO_CLANG
#pragma message "isConstantEvaluated() is dummy-implemented because no builtin function is available (clang)"
#endif
#ifdef VXIO_GNU
#pragma message "isConstantEvaluated() is dummy-implemented because no builtin function is available (gcc)"
#endif
#ifdef VXIO_MSVC
#pragma message "isConstantEvaluated() is dummy-implemented because no builtin function is available (msvc)"
#endif
    return true;
#endif
}

}  // namespace voxelio

#endif
