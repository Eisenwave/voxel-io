#include "voxelio/test/test.hpp"

#include "voxelio/color.hpp"
#include "voxelio/endian.hpp"

namespace voxelio::test {

VXIO_TEST(color32, channelsAndPredicatesCorrect)
{
    VXIO_STATIC_ASSERT_EQ(ArgbColor::RED.a, 0xFF);
    VXIO_STATIC_ASSERT_EQ(ArgbColor::RED.r, 0xFF);
    VXIO_STATIC_ASSERT_EQ(ArgbColor::RED.g, 0);
    VXIO_STATIC_ASSERT_EQ(ArgbColor::RED.b, 0);
    VXIO_ASSERT(ArgbColor::RED.isVisible());
    VXIO_ASSERT(ArgbColor::RED.isSolid());
    VXIO_ASSERT(not ArgbColor::RED.isTransparent());
    VXIO_ASSERT(not ArgbColor::RED.isInvisible());

    u8 bytes[4]{0x11, 0x22, 0x33, 0x44};
    Color32 rgb{bytes[0], bytes[1], bytes[2], bytes[3]};
    VXIO_ASSERT_EQ(rgb.a, 0x44);
    VXIO_ASSERT_EQ(rgb.r, 0x11);
    VXIO_ASSERT_EQ(rgb.g, 0x22);
    VXIO_ASSERT_EQ(rgb.b, 0x33);
    VXIO_ASSERT(rgb.isVisible());
    VXIO_ASSERT(not rgb.isSolid());
    VXIO_ASSERT(rgb.isTransparent());
    VXIO_ASSERT(not rgb.isInvisible());
}

VXIO_TEST(color32, decodeArgbCorrect)
{
    static constexpr u8 bytes[4]{'a', 'r', 'g', 'b'};

    constexpr auto makeArgb32 = [](u8 a, u8 r, u8 g, u8 b) -> argb32 {
        return Color32{r, g, b, a}.argb();
    };

    VXIO_ASSERT_EQ(voxelio::decodeArgb<voxelio::ArgbOrder::ARGB>(bytes), makeArgb32('a', 'r', 'g', 'b'));
    VXIO_ASSERT_EQ(voxelio::decodeArgb<voxelio::ArgbOrder::RGBA>(bytes), makeArgb32('r', 'g', 'b', 'a'));
    VXIO_ASSERT_EQ(voxelio::decodeArgb<voxelio::ArgbOrder::BGRA>(bytes), makeArgb32('b', 'g', 'r', 'a'));
}

}  // namespace voxelio::test
