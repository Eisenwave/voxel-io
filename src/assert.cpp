#include "voxelio/assert.hpp"

#ifndef VXIO_DISABLE_ASSERTS

#include "voxelio/log.hpp"

#include <vector>

namespace voxelio {

[[noreturn]] void defaultAssertHandler() noexcept
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

[[noreturn]] void assertFail(const std::string_view msg, SourceLocation loc) noexcept(false)
{
    std::string logMsg = "assertion error in " + std::string{loc.function} + std::string{"(): "} + std::string{msg};
    VXIO_LOG_IMPL(FAILURE, logMsg, loc.file, loc.function, loc.line);
    assertHandlerStack.back()();
    VXIO_UNREACHABLE();
}

}  // namespace voxelio

#endif
