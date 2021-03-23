#ifndef VXIO_TEST_HPP
#define VXIO_TEST_HPP

#include "voxelio/assert.hpp"
#include "voxelio/log.hpp"

#include <cstring>

namespace voxelio::test {

void pushLogLevel(LogLevel level);

void popLogLevel();

struct TestGuard {
    TestGuard() noexcept
    {
        pushAssertHandler([]() {
            throw nullptr;
        });
    }

    ~TestGuard() noexcept
    {
        popAssertHandler();
    }

    TestGuard(const TestGuard &) = delete;
    TestGuard(TestGuard &&) = delete;
};

struct Test {
public:
    const char *category;
    const char *name;

    explicit Test(const char *category, const char *name) : category{category}, name{name} {}
    virtual ~Test();

    bool operator()() const
    {
        TestGuard testGuard{};
        try {
            run();
            return true;
        }
        catch (...) {
            return false;
        }
    }

protected:
    virtual void run() const = 0;
};

namespace detail {

void registerTest(Test *test);

}  // namespace detail

void setTestOrder(const char *const prefixes[], usize count);

using TestConsumer = void(const Test &test);

void forEachTest(TestConsumer *action);

usize testCount();

#define VXIO_TEST_CLASS_NAME(name) Test_##name##_

#define VXIO_TEST(category, name)                                                                           \
    struct VXIO_TEST_CLASS_NAME(name) : ::voxelio::test::Test {                                             \
        [[maybe_unused]] VXIO_TEST_CLASS_NAME(name)() : ::voxelio::test::Test{#category, #name} {}          \
        void run() const final;                                                                             \
    };                                                                                                      \
    static const int name##__ = (::voxelio::test::detail::registerTest(new VXIO_TEST_CLASS_NAME(name)), 0); \
                                                                                                            \
    void VXIO_TEST_CLASS_NAME(name)::run() const

#ifndef DISABLE_STATIC_TESTS
#define VXIO_STATIC_ASSERT_EQ(x, y) \
    static_assert(x == y);          \
    VXIO_ASSERT_EQ(x, y)
#else
#define STATIC_ASSERT_EQ(x, y) TEST_ASSERT_EQ(x, y)
#endif

}  // namespace voxelio::test

#endif
