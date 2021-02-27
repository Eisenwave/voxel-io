#include "voxelio/hextree_.hpp"

#include "voxelio/ileave.hpp"

#include <queue>

namespace voxelio {

namespace {

// UTILITY =============================================================================================================

using Vec4u8 = detail::Vec4u8;

using detail::dileave4b;
using detail::ileave4b;
using detail::pack4b;
using detail::unpack4b;

constexpr u32 distanceSqr(Vec4u8 p0, Vec4u8 p1)
{
    Vec<i32, 4> distance = p0.cast<i32>() - p1;
    return static_cast<u32>(dot(distance, distance));
}

constexpr u32 distanceSqr(Vec4u8 p, Vec4u8 boxMin, Vec4u8 boxMax)
{
    Vec<u32, 4> distance{};
    for (usize i = 0; i < 4; ++i) {
        int d = std::max(int{boxMin.x()} - p.x(), int{p.x()} - boxMax.x());
        d = std::max(d, 0);
        VXIO_DEBUG_ASSERT_LT(d, 256);
        distance[i] = static_cast<u32>(d);
    }
    return dot(distance, distance);
}

}  // namespace

// HEX TREE IMPLEMENTATIONS ============================================================================================

void HexTree::insert(argb32 color, value_type value)
{
    const u32 morton = ileave4b(color);

    value_type *ptr = findOrCreate_impl<DEPTH>(morton, root, value);
    VXIO_DEBUG_ASSERT_NOTNULL(ptr);
    *ptr = value;
}

HexTree::value_type *HexTree::find(argb32 color)
{
    const u32 morton = ileave4b(color);

    return find_impl<DEPTH>(morton, root);
}

u32 HexTree::distanceSqr(Vec4u8 point)
{
    Vec4u8 closestPoint = closest(point).first;
    return voxelio::distanceSqr(closestPoint, point);
}

template <usize LEVEL>
HexTree::value_type *HexTree::find_impl(u32 morton, Node<LEVEL> &node)
{
    static_assert(LEVEL != 0);

    const u8 highestDigit = (morton >> 28) & 0xf;

    if (not node.has(highestDigit)) {
        return nullptr;
    }

    if constexpr (LEVEL > 1) {
        std::unique_ptr<Node<LEVEL - 1>> &child = node.children[highestDigit];
        VXIO_DEBUG_ASSERT_NOTNULL(child);

        return find_impl(morton << 4, *child);
    }
    else {
        return &node.values[highestDigit];
    }
}

template <usize LEVEL>
HexTree::value_type *HexTree::findOrCreate_impl(u32 morton, Node<LEVEL> &node, value_type paletteIndex)
{
    static_assert(LEVEL != 0);

    const u8 highestDigit = (morton >> 28) & 0xf;

    if constexpr (LEVEL > 1) {
        std::unique_ptr<Node<LEVEL - 1>> &child = node.children[highestDigit];
        if (not node.has(highestDigit)) {
            VXIO_DEBUG_ASSERT_NULL(child);
            child.reset(new Node<LEVEL - 1>{});
            node.add(highestDigit);
        }
        VXIO_DEBUG_ASSERT(node.has(highestDigit));

        return findOrCreate_impl(morton << 4, *child, paletteIndex);
    }
    else {
        node.add(highestDigit);
        return &node.values[highestDigit];
    }
}

std::pair<Vec4u8, HexTree::value_type> HexTree::closest(Vec4u8 point) const
{
    std::priority_queue<SearchEntry> queue;
    queue.push({{&root}, 0, 0, 0});  // root node

    SearchEntry closest{};
    closest.distance = ~u32{0};

    do {
        SearchEntry entry = queue.top();
        queue.pop();

        if (entry.distance >= closest.distance) {
            break;
        }

        if (entry.level == 0) {
            closest = entry;
            continue;
        }

        SearchEntry child;
        for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
            if (bool hasChild = childSearchEntry(point, entry, i, child); hasChild) {
                queue.push(child);
            }
        }

    } while (not queue.empty());

    VXIO_DEBUG_ASSERT_NE(closest.distance, ~u32{0});

    u32 closestPos = dileave4b(closest.morton);
    return {unpack4b(closestPos), closest.value};
}

bool HexTree::childSearchEntry(Vec4u8 p, const HexTree::SearchEntry &entry, u8 i, HexTree::SearchEntry &outChild)
{
    using ImplFunction = decltype(&HexTree::childSearchEntry);
    VXIO_DEBUG_ASSERT_NE(entry.level, 0);
    VXIO_DEBUG_ASSERT_LE(entry.level, DEPTH);

    constexpr ImplFunction functions[DEPTH]{
        &HexTree::childSearchEntry_impl<1>,
        &HexTree::childSearchEntry_impl<2>,
        &HexTree::childSearchEntry_impl<3>,
        &HexTree::childSearchEntry_impl<4>,
        &HexTree::childSearchEntry_impl<5>,
        &HexTree::childSearchEntry_impl<6>,
        &HexTree::childSearchEntry_impl<7>,
        &HexTree::childSearchEntry_impl<8>,
    };

    return functions[entry.level - 1](p, entry, i, outChild);
}

template <usize LEVEL>
bool HexTree::childSearchEntry_impl(Vec4u8 p, const HexTree::SearchEntry &entry, u8 i, HexTree::SearchEntry &outChild)
{
    constexpr usize childLevel = LEVEL - 1;
    VXIO_DEBUG_ASSERT_EQ(LEVEL, entry.level);
    VXIO_DEBUG_ASSERT_LT(i, BRANCHING_FACTOR);

    const auto *node = reinterpret_cast<const Node<LEVEL> *>(entry.node);
    if (not node->has(i)) {
        return false;
    }

    outChild.morton = (entry.morton << 4) | i;
    outChild.level = childLevel;

    const u32 min = dileave4b(outChild.morton << (childLevel * 4));
    const Vec4u8 minVec = unpack4b(min);

    if constexpr (LEVEL > 1) {
        const Vec4u8 size = Vec4u8::filledWith(1 << childLevel);
        const Vec4u8 maxVec = minVec + size - Vec4u8::one();
        VXIO_DEBUG_ASSERTM(maxVec == minVec.cast<u32>() + size, "Overflow");

        VXIO_DEBUG_ASSERT_NOTNULL(entry.node);

        outChild.node = node->children[i].get();
        outChild.distance = voxelio::distanceSqr(p, minVec, maxVec);
    }
    else {
        static_assert(LEVEL != 0);
        outChild.value = node->values[i];
        outChild.distance = voxelio::distanceSqr(p, minVec);
    }

    return true;
}

}  // namespace voxelio
