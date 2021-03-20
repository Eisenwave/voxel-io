#include "voxelio/assert.hpp"

#ifndef VXIO_DISABLE_ASSERTS

#include "voxelio/log.hpp"

#include <vector>

namespace voxelio {

[[noreturn]] void defaultAssertHandler()
{
    std::terminate();
}

static thread_local std::vector<AssertHandler> assertHandlerStack{&defaultAssertHandler};

void pushAssertHandler(AssertHandler handler) noexcept
{
    assertHandlerStack.push_back(handler);
}

void popAssertHandler() noexcept
{
    if (assertHandlerStack.size() <= 1) {
        VXIO_LOG(FAILURE, "Attempted to pop defaultAssertHandler");
        defaultAssertHandler();
    }
    assertHandlerStack.pop_back();
}

[[noreturn]] void assertFail(const char *file,
                             const unsigned line,
                             const char *function,
                             const std::string_view msg) noexcept(false)
{
    std::string logMsg = "assertion error in " + std::string{function} + std::string{"(): "} + std::string{msg};
    VXIO_LOG_IMPL(FAILURE, logMsg, file, function, line);
    assertHandlerStack.back()();
    VXIO_UNREACHABLE();
}

}  // namespace voxelio

#endif
