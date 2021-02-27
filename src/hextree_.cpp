#include "voxelio/hextree_.hpp"

#include "voxelio/ileave.hpp"

#include <queue>

namespace voxelio {

namespace {

// UTILITY =============================================================================================================

using Vec4u8 = detail::Vec4u8;

using detail::dileave4b;
using detail::distanceSqr;
using detail::ileave4b;
using detail::lengthSqr;
using detail::pack4b;
using detail::unpack4b;

constexpr u32 distanceSqr(Vec4u8 p, Vec4u8 boxMin, Vec4u8 boxMax)
{
    /// Buffer which is first used for point coordinates cast to int, then for the distance on each axis.
    Vec<int, 4> buffer = p.cast<int>();

    for (usize i = 0; i < 4; ++i) {
        const int d = std::max(boxMin[i] - buffer[i], buffer[i] - boxMax[i]);
        VXIO_DEBUG_ASSERT_LE(boxMin[i], boxMax[i]);
        VXIO_DEBUG_ASSERT_LT(d, 256);
        // negative distances may occur when the point is inside the box
        buffer[i] = std::max(d, 0);
    }

    return lengthSqr(buffer);
}

}  // namespace

// CLOSEST POINT SEARCH DETAILS ========================================================================================

namespace {

struct SearchEntry {
    union {
        const void *node;
        HexTree::value_type value;
    };

    SearchEntry (*childSearchEntryFunction)(const SearchEntry &, Vec4u8, u8);
    u32 morton;
    u32 distance;
    detail::HexTreeNodeBase nodeBase;
    u8 level;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
    constexpr bool operator<(const SearchEntry &rhs) const
    {
        return this->distance < rhs.distance;
    }

    constexpr bool operator>(const SearchEntry &rhs) const
    {
        return this->distance > rhs.distance;
    }
#pragma clang diagnostic pop

    SearchEntry childSearchEntry(Vec4u8 p, u8 i) const
    {
        VXIO_DEBUG_ASSERT_NOTNULL(childSearchEntryFunction);
        return childSearchEntryFunction(*this, p, i);
    }
};

template <usize LEVEL>
SearchEntry childSearchEntry_impl(const SearchEntry &entry, Vec4u8 p, u8 i)
{
    constexpr usize childLevel = LEVEL - 1;
    VXIO_DEBUG_ASSERT_EQ(LEVEL, entry.level);
    VXIO_DEBUG_ASSERT_LT(i, detail::HEX_TREE_BRANCHING_FACTOR);

    const auto *node = reinterpret_cast<const HexTree::Node<LEVEL> *>(entry.node);
    VXIO_DEBUG_ASSERT(node->has(i));

    SearchEntry outChild;
    outChild.morton = (entry.morton << 4) | i;
    outChild.level = childLevel;

    const u32 min = dileave4b(outChild.morton << (childLevel * 4));
    const Vec4u8 minVec = unpack4b(min);

    if constexpr (LEVEL > 1) {
        const Vec4u8 size = Vec4u8::filledWith(1 << childLevel);
        const Vec4u8 maxVec = minVec + size - Vec4u8::one();

        VXIO_DEBUG_ASSERTM(maxVec == (minVec.cast<u32>() + size - Vec<u32, 4>::one()), "Overflow");
        VXIO_DEBUG_ASSERT_NOTNULL(entry.node);

        const std::unique_ptr<HexTree::Node<childLevel>> &child = node->children[i];
        outChild.childSearchEntryFunction = &childSearchEntry_impl<childLevel>;
        outChild.nodeBase = *child;
        outChild.node = child.get();
        outChild.distance = voxelio::distanceSqr(p, minVec, maxVec);
    }
    else {
        static_assert(LEVEL != 0);
        outChild.childSearchEntryFunction = nullptr;
        // nodeBase is not assigned because there is no meaningful value for individual voxels
        outChild.value = node->values[i];
        outChild.distance = voxelio::distanceSqr(p, minVec);
    }

    return outChild;
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
    constexpr u32 rootMorton = 0;
    constexpr u32 rootDistance = 0;

    std::priority_queue<SearchEntry, std::vector<SearchEntry>, std::greater<SearchEntry>> queue;
    queue.push({{&root}, &childSearchEntry_impl<DEPTH>, rootMorton, rootDistance, root, DEPTH});

    SearchEntry closest{};
    closest.distance = ~u32{0};

    do {
        SearchEntry entry = queue.top();
        queue.pop();

        VXIO_DEBUG_ASSERT_NE(entry.level, 0);

        if (entry.distance >= closest.distance) {
            break;
        }

        if (entry.level > 1) {
            for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
                if (entry.nodeBase.has(i)) {
                    SearchEntry &&child = entry.childSearchEntry(point, i);
                    if (child.distance < closest.distance) {
                        queue.push(std::move(child));
                    }
                }
            }
        }
        else {
            for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
                if (entry.nodeBase.has(i)) {
                    SearchEntry child = childSearchEntry_impl<1>(entry, point, i);
                    VXIO_DEBUG_ASSERT_EQ(child.level, 0);

                    if (child.distance < closest.distance) {
                        closest = child;
                    }
                }
            }
        }

    } while (not queue.empty());

    VXIO_DEBUG_ASSERT_NE(closest.distance, ~u32{0});

    u32 closestPos = dileave4b(closest.morton);
    return {unpack4b(closestPos), closest.value};
}

}  // namespace voxelio
