#include "voxelio/palette.hpp"

#include "voxelio/assert.hpp"
#include "voxelio/color.hpp"
#include "voxelio/hextree_.hpp"
#include "voxelio/log.hpp"
#include "voxelio/vec.hpp"

#include <map>
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
using detail::pack4b;
using detail::unpack4b;

HexTree seedClusterCenters(argb32 colors[], usize colorCount, usize clusterCount)
{
    HexTree clusterCenters;
    if (clusterCount == 0) {
        return clusterCenters;
    }

    MovingAverage<u32, 16> avgDistance;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<u32> colorDistr{0, static_cast<u32>(colorCount - 1)};

    const argb32 firstCenter = colors[colorDistr(rng)];
    clusterCenters.insert(firstCenter, 0);
    VXIO_DEBUG_ASSERT(clusterCenters.contains(firstCenter));

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

struct Accumulator {
    Vec4u8 previousCenter;
    Vec<u32, 4> sum = {};
    u32 count = 0;
};

}  // namespace

std::unique_ptr<usize[]> Palette32::reduce(usize desiredSize, usize &outSize) const
{
    VXIO_LOG(DEBUG, "Starting palette reduction");
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

    // TODO change back after done debugging
    // std::unique_ptr<Accumulator[]> clusterAccumulators{new Accumulator[clusterCount]{}};
    std::vector<Accumulator> clusterAccumulators(clusterCount);
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
            VXIO_DEBUG_ASSERT(clusterCenters.contains(pack4b(closest.first)));
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
