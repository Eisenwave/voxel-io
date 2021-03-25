#include "voxelio/test/test.hpp"

#include "voxelio/intdiv.hpp"

namespace voxelio::test {
namespace {

VXIO_TEST(intdiv, divCeil)
{
    // uint / uint
    VXIO_STATIC_ASSERT_EQ(divCeil(0u, 2u), 0u);
    VXIO_STATIC_ASSERT_EQ(divCeil(2u, 1u), 2u);
    VXIO_STATIC_ASSERT_EQ(divCeil(3u, 2u), 2u);

    // int / uint
    VXIO_STATIC_ASSERT_EQ(divCeil(0, 2u), 0);
    VXIO_STATIC_ASSERT_EQ(divCeil(2, 1u), 2);
    VXIO_STATIC_ASSERT_EQ(divCeil(-3, 2u), -1);
    VXIO_STATIC_ASSERT_EQ(divCeil(3, 2u), 2);

    // uint / int
    VXIO_STATIC_ASSERT_EQ(divCeil(0u, 2), 0);
    VXIO_STATIC_ASSERT_EQ(divCeil(2u, 1), 2);
    VXIO_STATIC_ASSERT_EQ(divCeil(3u, -2), -1);
    VXIO_STATIC_ASSERT_EQ(divCeil(3u, 2), 2);

    // int / int
    VXIO_STATIC_ASSERT_EQ(divCeil(0, 2), 0);
    VXIO_STATIC_ASSERT_EQ(divCeil(2, 1), 2);
    VXIO_STATIC_ASSERT_EQ(divCeil(-3, -2), 2);
    VXIO_STATIC_ASSERT_EQ(divCeil(3, 2), 2);
    VXIO_STATIC_ASSERT_EQ(divCeil(-3, 2), -1);
    VXIO_STATIC_ASSERT_EQ(divCeil(3, -2), -1);
}

VXIO_TEST(intdiv, divFloor)
{
    // uint / uint
    VXIO_STATIC_ASSERT_EQ(divFloor(0u, 2u), 0u);
    VXIO_STATIC_ASSERT_EQ(divFloor(3u, 2u), 1u);

    // int / uint
    VXIO_STATIC_ASSERT_EQ(divFloor(0, 2u), 0);
    VXIO_STATIC_ASSERT_EQ(divFloor(-2, 1u), -2);
    VXIO_STATIC_ASSERT_EQ(divFloor(-3, 2u), -2);
    VXIO_STATIC_ASSERT_EQ(divFloor(3, 2u), 1);

    // uint / int
    VXIO_STATIC_ASSERT_EQ(divFloor(0u, 2), 0);
    VXIO_STATIC_ASSERT_EQ(divFloor(2u, 1), 2);
    VXIO_STATIC_ASSERT_EQ(divFloor(3u, -2), -2);
    VXIO_STATIC_ASSERT_EQ(divFloor(3u, 2), 1);

    // int / int
    VXIO_STATIC_ASSERT_EQ(divFloor(0, 2), 0);
    VXIO_STATIC_ASSERT_EQ(divFloor(-2, 1), -2);
    VXIO_STATIC_ASSERT_EQ(divFloor(-3, -2), 1);
    VXIO_STATIC_ASSERT_EQ(divFloor(3, 2), 1);
    VXIO_STATIC_ASSERT_EQ(divFloor(-3, 2), -2);
    VXIO_STATIC_ASSERT_EQ(divFloor(3, -2), -2);
}

}  // namespace
}  // namespace voxelio::test
