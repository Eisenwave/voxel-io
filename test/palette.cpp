#include "voxelio/test/random.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/color.hpp"
#include "voxelio/palette.hpp"

#include <map>
#include <random>

namespace voxelio::test {
namespace {

[[maybe_unused]] constexpr voxelio::Color32 BLACK = {uint8_t{0}, 0, 0};
[[maybe_unused]] constexpr voxelio::Color32 WHITE = {uint8_t{255u}, 255u, 255u};
[[maybe_unused]] constexpr voxelio::Color32 RED = {uint8_t{255u}, 0, 0};
[[maybe_unused]] constexpr voxelio::Color32 GREEN = {uint8_t{0}, 255u, 0};
[[maybe_unused]] constexpr voxelio::Color32 BLUE = {uint8_t{0}, 0, 255u};

VXIO_TEST(palette, reduce_2to1)
{
    voxelio::Palette32 palette;
    palette.insert(BLACK);
    palette.insert(WHITE);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(1, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, 1u);
    VXIO_ASSERT_EQ(reduced.size(), 1u);
}

VXIO_TEST(palette, reduce_4to2)
{
    voxelio::Palette32 palette;
    palette.insert(BLACK);
    palette.insert(WHITE);
    palette.insert(RED);
    palette.insert(BLUE);

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(2, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, 2u);
    VXIO_ASSERT_EQ(reduced.size(), 2u);
}

VXIO_TEST(palette, reduce_manyRandomTo256)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (9 + 4 * voxelio::build::RELEASE);

    voxelio::Palette32 palette;

    default_rng rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    for (uint32_t i = 0; i < colorCount; ++i) {
        palette.insert(distr(rng));
    }

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(desiredSize, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, desiredSize);
    VXIO_ASSERT_EQ(actualSize, reduced.size());
}

VXIO_TEST(palette, reduce_manyBlockTo256)
{
    constexpr size_t desiredSize = 256;
    constexpr size_t colorCount = size_t{1} << (12 + 4 * voxelio::build::RELEASE);

    voxelio::Palette32 palette;

    for (uint32_t i = 0; i < colorCount; ++i) {
        palette.insert(i);
    }

    size_t actualSize;
    std::unique_ptr<uint32_t[]> reduction = palette.reduce(desiredSize, actualSize);
    voxelio::Palette32 reduced = palette.createReducedPaletteAndStoreMapping(reduction.get());

    VXIO_ASSERT_EQ(actualSize, desiredSize);
    VXIO_ASSERT_EQ(actualSize, reduced.size());
}

}  // namespace
}  // namespace voxelio::test
