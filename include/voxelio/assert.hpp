#ifndef VXIO_ASSERT_HPP
#define VXIO_ASSERT_HPP

#include "build.hpp"
#include "builtin.hpp"
#include "stringify.hpp"

#include <string>

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
 * @brief Guard object which allows managing assert handlers RAII-style.
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

namespace voxelio {

[[noreturn]] void assertFail(std::string_view msg, SourceLocation location) noexcept(false);

}  // namespace voxelio

// The following definitions using ternary operators and the do ... while(false) pattern circumvent possible syntax
// breaks in if-else chains.
//
// For example,
//     if (...) ASSERT(...); else ...;
//
// ... would break if ASSERT was using an if-statement or regular code-block, because the following semicolon is then an
// additional statement, disconnecting the else-statement.
#define VXIO_ASSERT_IMPL(expr, msg) \
    (static_cast<bool>(expr) ? void(0) : ::voxelio::assertFail(msg, {__FILE__, __func__, __LINE__}))

#define VXIO_ASSERT_CMP(l, r, op)                                                                                   \
    do {                                                                                                            \
        auto &&tl_ = (l);                                                                                           \
        auto &&tr_ = (r);                                                                                           \
        VXIO_ASSERT_IMPL(tl_ op tr_,                                                                                \
                         "Comparison failed: " #l " " #op " " #r " (with \"" #l "\"=" + ::voxelio::stringify(tl_) + \
                             ", \"" #r "\"=" + ::voxelio::stringify(tr_) + ")");                                    \
    } while (false)

#define VXIO_ASSERT_CONSEQUENCE(l, r)                                                                           \
    do {                                                                                                        \
        auto &&tl_ = (l);                                                                                       \
        auto &&tr_ = (r);                                                                                       \
        VXIO_ASSERT_IMPL(!tl_ || tr_,                                                                           \
                         "Consequence failed: " #l " => " #r " (with \"" #l "\"=" + ::voxelio::stringify(tl_) + \
                             ", \"" #r "\"=" + ::voxelio::stringify(tr_) + ")");                                \
    } while (false)

#define VXIO_ASSERT_DIVISIBLE(l, r)                                                                             \
    do {                                                                                                        \
        auto &&tl_ = (l);                                                                                       \
        auto &&tr_ = (r);                                                                                       \
        VXIO_ASSERT_IMPL(tl_ % tr_ == 0,                                                                        \
                         "Divisibility failed: " #l " / " #r " (with \"" #l "\"=" + ::voxelio::stringify(tl_) + \
                             ", \"" #r "\"=" + ::voxelio::stringify(tr_) + ")");                                \
    } while (false)

#else

namespace voxelio::detail {
constexpr void consumeBool(bool) {}
}  // namespace voxelio::detail

#define VXIO_ASSERT_IMPL(expr, msg) ::voxelio::detail::consumeBool(expr)
#define VXIO_ASSERT_CMP(l, r, op) ::voxelio::detail::consumeBool((l) op(r))
#define VXIO_ASSERT_CONSEQUENCE(l, r) ::voxelio::detail::consumeBool(!(l) || (r))
#define VXIO_ASSERT_DIVISIBLE(l, r) ::voxelio::detail::consumeBool((l) % (r) == 0)

#endif
// COMMON ASSERTS (ALWAYS DEFINED THE SAME) ============================================================================

#define VXIO_ASSERT(...) VXIO_ASSERT_IMPL((__VA_ARGS__), "\"" #__VA_ARGS__ "\" evaluated to false")

#define VXIO_ASSERTM(expr, msg) VXIO_ASSERT_IMPL(expr, '"' + ::std::string{msg} + '"')
#define VXIO_ASSERT_FAIL(...) VXIO_ASSERTM(false, __VA_ARGS__)

#define VXIO_ASSERT_NOTNULL(...) VXIO_ASSERT_IMPL((__VA_ARGS__) != nullptr, #__VA_ARGS__ " must never be null")
#define VXIO_ASSERT_NULL(...) VXIO_ASSERT_IMPL((__VA_ARGS__) == nullptr, #__VA_ARGS__ " must always be null")
#define VXIO_ASSERT_EQ(l, r) VXIO_ASSERT_CMP(l, r, ==)
#define VXIO_ASSERT_NE(l, r) VXIO_ASSERT_CMP(l, r, !=)
#define VXIO_ASSERT_LT(l, r) VXIO_ASSERT_CMP(l, r, <)
#define VXIO_ASSERT_LE(l, r) VXIO_ASSERT_CMP(l, r, <=)
#define VXIO_ASSERT_GT(l, r) VXIO_ASSERT_CMP(l, r, >)
#define VXIO_ASSERT_GE(l, r) VXIO_ASSERT_CMP(l, r, >=)

#define VXIO_DEBUG_ASSERT(...) VXIO_IF_DEBUG(VXIO_ASSERT(__VA_ARGS__))
#define VXIO_DEBUG_ASSERTM(expr, msg) VXIO_IF_DEBUG(VXIO_ASSERTM(expr, msg))
#define VXIO_DEBUG_ASSERT_FAIL(...) VXIO_IF_DEBUG(VXIO_ASSERT_FAIL(__VA_ARGS__))
#define VXIO_DEBUG_ASSERT_NOTNULL(...) VXIO_IF_DEBUG(VXIO_ASSERT_NOTNULL(__VA_ARGS__))
#define VXIO_DEBUG_ASSERT_NULL(...) VXIO_IF_DEBUG(VXIO_ASSERT_NULL(__VA_ARGS__))
#define VXIO_DEBUG_ASSERT_EQ(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, ==))
#define VXIO_DEBUG_ASSERT_NE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, !=))
#define VXIO_DEBUG_ASSERT_LT(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, <))
#define VXIO_DEBUG_ASSERT_LE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, <=))
#define VXIO_DEBUG_ASSERT_GT(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, >))
#define VXIO_DEBUG_ASSERT_GE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CMP(l, r, >=))
#define VXIO_DEBUG_ASSERT_CONSEQUENCE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_CONSEQUENCE(l, r))
#define VXIO_DEBUG_ASSERT_DIVISIBLE(l, r) VXIO_IF_DEBUG(VXIO_ASSERT_DIVISIBLE(l, r))

// UNREACHABILITY ASSERTS ==============================================================================================
//
// This section provides VXIO_ASSERT_UNREACHABLE() and VXIO_DEBUG_ASSERT_UNREACHABLE()
// The former is simply an assertion which always fails, the latter only fails on debug builds.
// In situations where the assertion does not fail (VXIO_DISABLE_ASSERTS or VXIO_RELASE), VXIO_UNREACHABLE() is used.
// This indicates to the compiler that the code section is not reachable using a builtin, if available.

#ifndef VXIO_DISABLE_ASSERTS

#define VXIO_ASSERT_UNREACHABLE() VXIO_ASSERT_IMPL(false, "This execution path must be unreachable")

#if defined(VXIO_GNU) && (VXIO_GNU <= 8)
// We need to create a separate define for g++8 and lower.
// Otherwise, constexpr functions won't compile, even if the assert failure is in an unreachable line of code.
// For example, a funcion like:
//      constexpr void f() {return; call_non_constexpr_function();}
// will not compile using g++8 because a non-constexpr function is called.
// In g++9 and upwards, the problem is fixed.
// The best we can do is indicate to the compiler that the code section is unreachable.
#define VXIO_DEBUG_ASSERT_UNREACHABLE() VXIO_UNREACHABLE()
#else
// In all other cases, such as any clang version and g++9 we can just fail in unreachable sections.
#define VXIO_DEBUG_ASSERT_UNREACHABLE() VXIO_IF_DEBUG_ELSE(VXIO_ASSERT_UNREACHABLE(), VXIO_UNREACHABLE())
#endif

#else

#define VXIO_ASSERT_UNREACHABLE() VXIO_UNREACHABLE()
#define VXIO_DEBUG_ASSERT_UNREACHABLE() VXIO_UNREACHABLE()

#endif
// =====================================================================================================================

#endif  // ASSERT_HPP
