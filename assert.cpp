#include "assert.hpp"

#ifndef VXIO_DISABLE_ASSERTS

#include <iostream>
#include <vector>

namespace voxelio {

[[noreturn]] void defaultAssertHandler()
{
    std::terminate();
}

static thread_local std::vector<AssertHandler> assertHandlerStack{defaultAssertHandler};

void pushAssertHandler(AssertHandler handler) noexcept
{
    assertHandlerStack.push_back(handler);
}

void popAssertHandler() noexcept
{
    if (assertHandlerStack.size() <= 1) {
        std::cerr << "Attempted to pop defaultAssertHandler" << '\n';
        defaultAssertHandler();
    }
    assertHandlerStack.pop_back();
}

[[noreturn]] void assertFail(const char *file,
                             const unsigned line,
                             const char *function,
                             const std::string_view msg) noexcept(false)
{
    std::cerr << file << ':' << line << ": assertion error in " << function << "(): " << msg << '\n';
    assertHandlerStack.back()();
    VXIO_UNREACHABLE();
}

}  // namespace voxelio

#endif
