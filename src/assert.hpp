#ifndef VXIO_ASSERT_HPP
#define VXIO_ASSERT_HPP

#include "build.hpp"
#include "builtin.hpp"

// USER CONFIG =========================================================================================================
//#define VXIO_DISABLE_ASSERTS

// ASSERTION HANDLING ==================================================================================================

namespace voxelio {
using AssertHandler = void (*)();

#ifndef VXIO_DISABLE_ASSERTS
/**
 * @brief Pushes an AssertHandler onto the handler stack.
 * Until this handler is popped, it will be invoked every time an assertion fails.
 * @param handler the handler
 */
void pushAssertHandler(AssertHandler handler) noexcept;

/**
 * @brief Pops an AssertHandler from the stack.
 * If no handler can be popped, the default assertion handler is invoked, which terminates the program.
 * @param handler the handler
 */
void popAssertHandler() noexcept;
#else
inline void pushAssertHandler(AssertHandler) noexcept {}
inline void popAssertHandler() noexcept {}
#endif

/**
 * @brief Guard object which allows managing assert handles RAII-style.
 */
struct AssertHandlerGuard {
    AssertHandlerGuard(AssertHandler handler) noexcept
    {
        pushAssertHandler(handler);
    }
    ~AssertHandlerGuard() noexcept
    {
        popAssertHandler();
    }

    AssertHandlerGuard(const AssertHandlerGuard &) = delete;
    AssertHandlerGuard(AssertHandlerGuard &&) = delete;
};
}  // namespace voxelio

// CONDITIONALLY DISABLED ASSSERTS =====================================================================================
#ifndef VXIO_DISABLE_ASSERTS
#include "stringify.hpp"

namespace voxelio {

[[noreturn]] void assertFail(const char *file,
                             const unsigned line,
                             const char *function,
                             const std::string_view msg) noexcept(false);

}  // namespace voxelio

// this is not defined as a ternary operator so that it doesn't break an else-statement
#define VXIO_ASSERT_IMPL(expr, msg) \
    (static_cast<bool>(expr) ? void(0) : ::voxelio::assertFail(__FILE__, __LINE__, __func__, msg))

#define VXIO_TO_STRING(x) ::voxelio::stringify(x)

#define VXIO_ASSERT_CMP(l, r, op)                                                                       \
    VXIO_ASSERT_IMPL((l) op(r),                                                                         \
                     "Comparison failed: " #l " " #op " " #r " (with \"" #l "\"=" + VXIO_TO_STRING(l) + \
                         ", \"" #r "\"=" + VXIO_TO_STRING(r) + ")")

#define VXIO_ASSERT_CONSEQUENCE(l, r)                                                               \
    VXIO_ASSERT_IMPL(!(l) || (r),                                                                   \
                     "Consequence failed: " #l " => " #r " (with \"" #l "\"=" + VXIO_TO_STRING(l) + \
                         ", \"" #r "\"=" + VXIO_TO_STRING(r) + ")")

#else

namespace voxelio::detail {
constexpr void consumeBool(bool) {}
}  // namespace voxelio::detail

#define VXIO_ASSERT_IMPL(expr, msg) ::voxelio::detail::consumeBool(expr)
#define VXIO_ASSERT_CMP(l, r, op) ::voxelio::detail::consumeBool((l) op(r))
#define VXIO_ASSERT_CONSEQUENCE(l, r) ::voxelio::detail::consumeBool(!(l) || (r))

#endif
// COMMON ASSERTS (ALWAYS DEFINED THE SAME) ============================================================================

#define VXIO_ASSERT(expr) VXIO_ASSERT_IMPL(expr, "\"" #expr "\" evaluated to false")

#define VXIO_ASSERTM(expr, msg) VXIO_ASSERT_IMPL(expr, '"' + ::std::string{msg} + '"')
#define VXIO_ASSERT_FAIL(msg) VXIO_ASSERTM(false, msg)

#define VXIO_ASSERT_NOTNULL(expr) VXIO_ASSERT_IMPL(expr != nullptr, #expr " must never be null")
#define VXIO_ASSERT_NULL(expr) VXIO_ASSERT_IMPL(expr == nullptr, #expr " must always be null")
#define VXIO_ASSERT_EQ(l, r) VXIO_ASSERT_CMP(l, r, ==)
#define VXIO_ASSERT_NE(l, r) VXIO_ASSERT_CMP(l, r, !=)
#define VXIO_ASSERT_LT(l, r) VXIO_ASSERT_CMP(l, r, <)
#define VXIO_ASSERT_LE(l, r) VXIO_ASSERT_CMP(l, r, <=)
#define VXIO_ASSERT_GT(l, r) VXIO_ASSERT_CMP(l, r, >)
#define VXIO_ASSERT_GE(l, r) VXIO_ASSERT_CMP(l, r, >=)

#define VXIO_DEBUG_ASSERT(expr) VXIO_IF_DEBUG(VXIO_ASSERT(expr))
#define VXIO_DEBUG_ASSERTM(expr, msg) VXIO_IF_DEBUG(VXIO_ASSERTM(expr, msg))
#define VXIO_DEBUG_ASSERT_FAIL(msg) VXIO_IF_DEBUG(VXIO_ASSERT_FAIL(msg))
#define VXIO_DEBUG_ASSERT_NOTNULL(expr) VXIO_IF_DEBUG(VXIO_ASSERT_NOTNULL(expr))
#define VXIO_DEBUG_ASSERT_NULL(expr) VXIO_IF_DEBUG(VXIO_ASSERT_NULL(expr))
#define VXIO_DEBUG_ASSERT_EQ(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, ==))
#define VXIO_DEBUG_ASSERT_NE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, !=))
#define VXIO_DEBUG_ASSERT_LT(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, <))
#define VXIO_DEBUG_ASSERT_LE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, <=))
#define VXIO_DEBUG_ASSERT_GT(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, >))
#define VXIO_DEBUG_ASSERT_GE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, >=))
#define VXIO_DEBUG_ASSERT_CONSEQUENCE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CONSEQUENCE(l, r))

// UNREACHABILITY ASSERTS ==============================================================================================
//
// This section provides VXIO_ASSERT_UNREACHABLE() and VXIO_DEBUG_ASSERT_UNREACHABLE()
// The former is simply an assertion which always fails, the latter only fails on debug builds.
// In situations where the assertion does not fail (VXIO_DISABLE_ASSERTS or VXIO_RELASE), VXIO_UNREACHABLE() is used.
// This indicates to the compiler that the code section is not reachable using a builtin, if available.

#ifndef VXIO_DISABLE_ASSERTS

#define VXIO_ASSERT_UNREACHABLE() VXIO_ASSERT_IMPL(false, "This execution path must be unreachable")
#define VXIO_DEBUG_ASSERT_UNREACHABLE() VXIO_IF_DEBUG_ELSE(VXIO_ASSERT_UNREACHABLE(), VXIO_UNREACHABLE())

#else

#define VXIO_ASSERT_UNREACHABLE() VXIO_UNREACHABLE()
#define VXIO_DEBUG_ASSERT_UNREACHABLE() VXIO_UNREACHABLE()

#endif
// =====================================================================================================================

#endif  // ASSERT_HPP
