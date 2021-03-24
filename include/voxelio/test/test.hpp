#ifndef VXIO_TEST_HPP
#define VXIO_TEST_HPP

#include "voxelio/assert.hpp"
#include "voxelio/log.hpp"

#include <cstring>

namespace voxelio::test {

struct Test {
private:
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

public:
    const char *category;
    const char *name;

    explicit Test(const char *category, const char *name) : category{category}, name{name} {}
    virtual ~Test();

    bool operator()() const
    {
        TestGuard testGuard;
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

using TestConsumer = void(const Test &test);

namespace detail {

void registerTest(Test *test);

}  // namespace detail

void setTestOrder(const char *const prefixes[], usize count);

void forEachTest(TestConsumer *action);

usize getTestCount();

#define VXIO_TEST_CLASS_NAME(category, name) Test_##category##_##name##_

#define VXIO_TEST(category, name)                                                                            \
    struct VXIO_TEST_CLASS_NAME(category, name) : ::voxelio::test::Test {                                    \
        [[maybe_unused]] VXIO_TEST_CLASS_NAME(category, name)() : ::voxelio::test::Test{#category, #name} {} \
        void run() const final;                                                                              \
    };                                                                                                       \
    static const int category##_##name##_ =                                                                  \
        (::voxelio::test::detail::registerTest(new VXIO_TEST_CLASS_NAME(category, name)), 0);                \
                                                                                                             \
    void VXIO_TEST_CLASS_NAME(category, name)::run() const

#ifndef DISABLE_STATIC_TESTS
#define VXIO_STATIC_ASSERT_EQ(x, y) \
    static_assert(x == y);          \
    VXIO_ASSERT_EQ(x, y)
#else
#define STATIC_ASSERT_EQ(x, y) TEST_ASSERT_EQ(x, y)
#endif

}  // namespace voxelio::test

#endif
