#include "voxelio/test/test.hpp"

namespace voxelio::test {

VXIO_TEST(assert, noMultipleEvaluations_cmp)
{
    int x = 0;
    int y = 0;

    VXIO_ASSERT_EQ(++x, ++y);
    VXIO_ASSERT_EQ(x, 1);
    VXIO_ASSERT_EQ(y, 1);
}

VXIO_TEST(assert, noMultipleEvaluations_divisible)
{
    int x = 0;
    int y = 0;

    VXIO_ASSERT_DIVISIBLE(++x, ++y);
    VXIO_ASSERT_EQ(x, 1);
    VXIO_ASSERT_EQ(y, 1);
}

VXIO_TEST(assert, noMultipleEvaluations_consequence)
{
    int x = 0;
    int y = 0;

    VXIO_ASSERT_CONSEQUENCE(++x, ++y);
    VXIO_ASSERT_EQ(x, 1);
    VXIO_ASSERT_EQ(y, 1);
}

VXIO_TEST(assert, noUnnecessaryCopies)
{
    struct S {
        mutable int value;
        S(int v) : value{v} {}
        S() = delete;
        S(const S &) = delete;
        S(S &&) = delete;

        bool operator==(const S &) const
        {
            return true;
        }
        int operator%(const S &other) const
        {
            return value % other.value;
        }
        explicit operator bool() const
        {
            ++value;
            return true;
        }
        [[maybe_unused]] explicit operator std::string() const
        {
            return stringify(value);
        }
    };

    S x{0};
    S y{1};

    VXIO_ASSERT_EQ(x, y);
    VXIO_ASSERT_DIVISIBLE(x, y);
    VXIO_ASSERT_CONSEQUENCE(x, y);

    VXIO_ASSERT_EQ(x.value, 1);
    VXIO_ASSERT_EQ(y.value, 2);
}

}  // namespace voxelio::test
