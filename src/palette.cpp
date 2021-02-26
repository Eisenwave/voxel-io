#include "voxelio/palette.hpp"

#include "voxelio/assert.hpp"
#include "voxelio/color.hpp"
#include "voxelio/ileave.hpp"
#include "voxelio/vec.hpp"

#include <map>
#include <queue>
#include <random>
#include <vector>

namespace voxelio {

u32 Palette32::insert(argb32 color)
{
    auto iter = colorToIndexMap.find(color);
    if (iter != colorToIndexMap.end()) {
        return iter->second;
    }
    return insertUnsafe(color);
}

u32 Palette32::insertUnsafe(argb32 color)
{
    auto result = static_cast<u32>(size());
    colorToIndexMap.emplace(color, result);
    return result;
}

std::unique_ptr<argb32[]> Palette32::build() const
{
    auto size = colorToIndexMap.size();
    auto result = std::make_unique<argb32[]>(size);
    for (auto [color, index] : colorToIndexMap) {
        VXIO_DEBUG_ASSERT_LT(index, size);
        result[index] = color;
    }
    return result;
}

namespace {

using Vec4u8 = Vec<u8, 4>;

constexpr u32 ileave4b(u32 bytes)
{
    return static_cast<u32>(ileaveBytes_const<4>(bytes));
    ;
}

constexpr u32 dileave4b(u32 bytes)
{
    return static_cast<u32>(dileaveBytes_const<4>(bytes));
    ;
}

constexpr u32 pack4b(Vec4u8 v)
{
    return (u32{v[0]} << 24) | (u32{v[1]} << 16) | (u32{v[2]} << 8) | (u32{v[3]} << 0);
}

constexpr Vec4u8 unpack4b(u32 b)
{
    return Vec<u32, 4>{b >> 24, b >> 16, b >> 8, b >> 0}.cast<u8>();
}

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

// HEX TREE ============================================================================================================

struct HexTree {
private:
    constexpr static usize DEPTH = 8;
    constexpr static usize BRANCHING_FACTOR = 16;

    using value_type = u32;

    struct SearchEntry {
        union {
            const void *node;
            value_type value;
        };

        u32 morton;
        u32 distance;
        u8 level;

        constexpr bool operator<(const SearchEntry &rhs)
        {
            return this->distance < rhs.distance;
        }
    };

    struct NodeBase {
        u16 childMask = 0;

        bool has(usize i) const
        {
            return getBit(childMask, i);
        }

        void add(usize i)
        {
            childMask = setBit(childMask, i);
        }
    };

    template <usize LEVEL>
    struct Node : public NodeBase {
        static_assert(LEVEL != 0);

        std::unique_ptr<Node<LEVEL - 1>> children[BRANCHING_FACTOR];
    };

    template <>
    struct Node<1> : public NodeBase {
        value_type values[BRANCHING_FACTOR];
    };

    Node<DEPTH> root;

public:
    /**
     * @brief Inserts a color into the HexTree.
     * @param color the color
     * @param value the value of the color (palette index)
     */
    void insert(argb32 color, value_type value)
    {
        const u32 morton = ileave4b(color);

        value_type *ptr = findOrCreate_impl<DEPTH>(morton, root, value);
        VXIO_DEBUG_ASSERT_NOTNULL(ptr);
        *ptr = value;
    }

    value_type *find(argb32 color)
    {
        const u32 morton = ileave4b(color);

        return find_impl<DEPTH>(morton, root);
    }

    bool contains(argb32 color)
    {
        return find(color) != nullptr;
    }

    template <typename F>
    void forEach(F action)
    {
        forEach_impl(0, &root, std::move(action));
    }

    /**
     * @brief Finds the closest color and its value to a given point.
     * @param point the query point
     * @return the closest color and its value
     */
    std::pair<Vec4u8, value_type> closest(Vec4u8 point) const;

    /**
     * @brief Returns the distance of a given point to any point in the tree.
     * This is equivalent to the distance to the closest point.
     * @param point the query point
     * @return the distance to the closest point
     */
    u32 distanceSqr(Vec4u8 point)
    {
        Vec4u8 closestPoint = closest(point).first;
        return voxelio::distanceSqr(closestPoint, point);
    }

private:
    template <usize LEVEL>
    value_type *find_impl(u32 morton, Node<LEVEL> &node);

    template <usize LEVEL>
    value_type *findOrCreate_impl(u32 morton, Node<LEVEL> &node, value_type paletteIndex);

    template <typename F, usize LEVEL>
    void forEach_impl(u32 morton, Node<LEVEL> &node, F action);

    static bool childSearchEntry(Vec4u8 p, const SearchEntry &entry, u8 i, SearchEntry &outChild);

    template <usize LEVEL>
    static bool childSearchEntry_impl(Vec4u8 p, const SearchEntry &entry, u8 i, SearchEntry &outChild);
};

// HEX TREE IMPLEMENTATIONS ============================================================================================

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

template <typename F, usize LEVEL>
void HexTree::forEach_impl(u32 morton, Node<LEVEL> &node, F action)
{
    for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
        if (not node.has(i)) {
            continue;
        }
        const u32 childMorton = (morton << 4) | i;
        if constexpr (LEVEL > 1) {
            forEach_impl<LEVEL - 1>(childMorton, *node.children[i], action);
        }
        else {
            u32 childPos = dileave4b(childMorton);
            action(childPos, node.values[i]);
        }
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

        if (entry.level != 0) {
            SearchEntry child;
            for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
                if (bool hasChild = childSearchEntry(point, entry, i, child); hasChild) {
                    queue.push(child);
                }
            }
        }
        else {
            closest.morton = entry.morton;
            closest.value = closest.distance = entry.distance;
        }
    } while (not queue.empty());

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

HexTree seedClusterCenters(argb32 colors[], usize colorCount, usize clusterCount)
{
    HexTree clusterCenters;
    if (clusterCount == 0) {
        return clusterCenters;
    }

    MovingAverage<u32, 16> avgDistance;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<u32> colorDistr{0, static_cast<u32>(colorCount - 1)};

    clusterCenters.insert(colors[colorDistr(rng)], 0);

    for (u32 clusterIndex = 1; clusterIndex < clusterCount; ++clusterIndex) {
        do {
            const u32 randomColorIndex = colorDistr(rng);
            const argb32 randomColor = colors[randomColorIndex];
            if (clusterCenters.contains(randomColor)) {
                continue;
            }
            const u32 distanceToOtherClusters = clusterCenters.distanceSqr(unpack4b(randomColor));
            avgDistance += distanceToOtherClusters;
            VXIO_DEBUG_ASSERT_NE(distanceToOtherClusters, 0);

            std::uniform_int_distribution<u32> rejectDistr{0, *avgDistance * 2};

            if (u32 randomRejection = rejectDistr(rng); randomRejection > distanceToOtherClusters) {
                continue;
            }

            clusterCenters.insert(randomColor, clusterIndex);
            break;
        } while (true);
    }

    return clusterCenters;
}

}  // namespace

std::unique_ptr<usize[]> Palette32::reduce(usize desiredSize, usize &outSize) const
{
    const usize clusterCount = std::min(this->size(), desiredSize);
    const usize colorCount = size();
    outSize = clusterCount;

    auto result = std::make_unique<usize[]>(colorCount);

    // Return identity mapping if no reduction is necessary.
    if (clusterCount == colorCount) {
        for (usize i = 0; i < colorCount; ++i) {
            result[i] = i;
        }
        return result;
    }

    const std::unique_ptr<argb32[]> colors = build();

    /// Maps from cluster center points to a unique index of the center.
    HexTree clusterCenters = seedClusterCenters(colors.get(), colorCount, clusterCount);

    struct Accumulator {
        Vec4u8 previousCenter;
        Vec<u32, 4> sum = {};
        u32 count = 0;
    };

    std::unique_ptr<Accumulator[]> clusterAccumulators{new Accumulator[clusterCount]{}};
    clusterCenters.forEach([&clusterAccumulators](Vec4u8 center, u32 index) -> void {
        clusterAccumulators[index].previousCenter = center;
    });

    while (true) {
        bool anyChange = false;

        // 1. Find all closest clusters, accumulate sums of points belonging to clusters for new center computation
        for (usize i = 0; i < colorCount; ++i) {
            argb32 color = colors[i];
            Vec4u8 point = unpack4b(color);
            std::pair<Vec4u8, u32> closest = clusterCenters.closest(point);
            VXIO_DEBUG_ASSERT_LT(closest.second, clusterCount);
            Accumulator &acc = clusterAccumulators[closest.second];
            acc.sum += point.cast<u32>();
            acc.count += 1;
        }

        // 2. Compute new cluster centers and reset accumulators
        HexTree newCenters;
        for (u32 clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
            Accumulator &acc = clusterAccumulators[clusterIndex];
            const Vec<u32, 4> center = acc.sum / acc.count;
            if constexpr (build::DEBUG) {
                for (usize i = 0; i < 4; ++i) {
                    VXIO_DEBUG_ASSERT_LT(center[i], 256);
                }
            }
            const Vec4u8 center8 = center.cast<u8>();
            newCenters.insert(pack4b(center8), clusterIndex);
            anyChange |= center8 != acc.previousCenter;
            acc = {center8, {}, 0};
        }
        clusterCenters = std::move(newCenters);

        // 3. Repeat until convergence
        if (not anyChange) {
            break;
        }
    }

    HexTree points;
    for (u32 i = 0; i < colorCount; ++i) {
        points.insert(colors[i], i);
    }

    for (usize i = 0; i < colorCount; ++i) {
        argb32 color = colors[i];
        Vec4u8 point = unpack4b(color);
        std::pair<Vec4u8, u32> closestClusterCenter = clusterCenters.closest(point);
        // TODO consider caching the representative for each center instead of computing it here
        std::pair<Vec4u8, u32> representativeColor = points.closest(closestClusterCenter.first);

        result[i] = representativeColor.second;
    }

    return result;
}

}  // namespace voxelio
