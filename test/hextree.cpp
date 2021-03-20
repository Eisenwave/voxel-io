#include "voxelio/test/random.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/color.hpp"
#include "voxelio/hextree_.hpp"

#include <map>
#include <random>

namespace voxelio::test {
namespace {

VXIO_TEST(hextree, insertedElementsAreFindable)
{
    constexpr size_t iterations = 1024 * 8;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    std::map<uint32_t, uint32_t> map;
    voxelio::HexTree tree;

    for (uint32_t i = 0; i < iterations; ++i) {
        uint32_t pos = distr(rng);

        map.emplace(pos, i);
        tree.insert(pos, i);
    }

    for (auto [pos, index] : map) {
        VXIO_ASSERT(tree.contains(pos));
        VXIO_ASSERT_EQ(*tree.find(pos), index);
    }
}

VXIO_TEST(hextree, onlyInsertedElementsAreFindable)
{
    constexpr size_t iterations = 1024 * 8;

    default_rng rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    std::map<uint32_t, uint32_t> map;
    voxelio::HexTree tree;

    for (uint32_t i = 0; i < iterations; ++i) {
        uint32_t pos = distr(rng);

        map.emplace(pos, i);
        tree.insert(pos, i);
    }

    for (uint32_t i = 0; i < iterations; ++i) {
        uint32_t pos = distr(rng);

        if (map.find(pos) == map.end()) {
            VXIO_ASSERT(not tree.contains(pos));
        }
    }
}

VXIO_TEST(hextree, forEachIteratesOverAll)
{
    constexpr size_t iterations = 1024 * 8;
    constexpr size_t expectedInsertions = iterations * 9999 / 10000;

    default_rng rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    voxelio::HexTree tree;

    size_t expectedCount = 0;

    for (uint32_t i = 0; i < iterations; ++i) {
        uint32_t pos = distr(rng);

        expectedCount += not tree.contains(pos);
        tree.insert(pos, i);
    }

    // We expect that at only a couple thousand iterations, almost all insertions are successful.
    VXIO_ASSERT(expectedCount >= expectedInsertions);

    size_t actualCount = 0;
    tree.forEach([&actualCount, &tree](uint32_t color, uint32_t iterIndex) -> void {
        ++actualCount;

        uint32_t *index = tree.find(color);

        VXIO_ASSERTM(index != nullptr, "forEach() iterates over point not in HexTree");
        VXIO_ASSERT_EQ(*index, iterIndex);
    });

    VXIO_ASSERT_EQ(expectedCount, actualCount);
}

VXIO_TEST(hextree, closestPointManual)
{
    constexpr auto black = voxelio::Vec<uint32_t, 4>{0, 0, 0, 0}.cast<uint8_t>();
    constexpr auto white = voxelio::Vec<uint32_t, 4>{255, 255, 255, 255}.cast<uint8_t>();
    constexpr auto some = voxelio::Vec<uint32_t, 4>{16, 55, 77, 200}.cast<uint8_t>();

    constexpr uint32_t black_packed = voxelio::detail::pack4b(black);
    constexpr uint32_t white_packed = voxelio::detail::pack4b(white);
    constexpr uint32_t some_packed = voxelio::detail::pack4b(some);

    default_rng rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    voxelio::HexTree tree;

    tree.insert(black_packed, 0);
    tree.insert(white_packed, 1);
    tree.insert(some_packed, 2);

    uint32_t closestToSome = tree.closest(some_packed).first;
    uint32_t closestToWhite = tree.closest(white_packed).first;
    uint32_t closestToBlack = tree.closest(black_packed).first;

    VXIO_ASSERT_EQ(black_packed, closestToBlack);
    VXIO_ASSERT_EQ(white_packed, closestToWhite);
    VXIO_ASSERT_EQ(some_packed, closestToSome);
}

VXIO_TEST(hextree, eachPointIsItsOwnClosest)
{
    constexpr size_t iterations = 1024 * 8;

    default_rng rng{12345};
    std::uniform_int_distribution<uint32_t> distr;

    voxelio::HexTree tree;

    for (uint32_t i = 0; i < iterations; ++i) {
        uint32_t pos = distr(rng);
        tree.insert(pos, i);
    }

    tree.forEach([&tree](uint32_t color, uint32_t index) -> void {
        std::pair<uint32_t, uint32_t> closest = tree.closest(color);

        VXIO_ASSERTM(tree.contains(closest.first), "Hextree doesn't contain closest found");
        VXIO_ASSERTM(color == closest.first, "Point is not its own closest");
        VXIO_ASSERT_EQ(index, closest.second);
    });
}

}  // namespace
}  // namespace voxelio::test
