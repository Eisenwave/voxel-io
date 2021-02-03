#ifndef VXIO_LANGEXT_HPP
#define VXIO_LANGEXT_HPP

#include "builtin.hpp"

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

}  // namespace voxelio

#endif
