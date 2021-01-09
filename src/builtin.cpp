#include "voxelio/builtin.hpp"

#include <exception>

namespace voxelio::builtin {

#ifndef VXIO_HAS_BUILTIN_TRAP
[[noreturn]] void trap()
{
    std::terminate();
}
#endif

}  // namespace voxelio::builtin
