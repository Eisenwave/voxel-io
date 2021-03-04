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

namespace {

using Vec4u8 = Vec<u8, 4>;
using detail::pack4b;
using detail::unpack4b;

HexTree seedClusterCenters(const argb32 colors[], usize colorCount, usize clusterCount) noexcept
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

std::unique_ptr<u32[]> identityReduction(usize colorCount)
{
    auto result = std::make_unique<u32[]>(colorCount);
    for (u32 i = 0; i < colorCount; ++i) {
        result[i] = i;
    }
    return result;
}

struct Accumulator {
    Vec4u8 previousCenter;
    Vec<u32, 4> sum = {};
    u32 count = 0;
};

}  // namespace

std::unique_ptr<u32[]> Palette32::reduce(usize desiredSize, usize &outSize) const
{
    VXIO_LOG(DETAIL, "Starting palette reduction");
    const usize clusterCount = std::min(this->size(), desiredSize);
    const usize colorCount = size();
    outSize = clusterCount;

    // Return identity mapping if no reduction is necessary.
    if (clusterCount == colorCount) {
        return identityReduction(colorCount);
    }

    auto result = std::make_unique<u32[]>(colorCount);

    /// Maps from cluster center points to a unique index of the center.
    HexTree clusterCenters = seedClusterCenters(data(), colorCount, clusterCount);

    std::unique_ptr<Accumulator[]> clusterAccumulators{new Accumulator[clusterCount]{}};
    clusterCenters.forEach([&clusterAccumulators](Vec4u8 center, u32 index) -> void {
        clusterAccumulators[index].previousCenter = center;
    });

    usize iterations = 0;
    bool anyChange;

    do {
        u64 totalChange = 0;
        anyChange = false;
        ++iterations;

        // 1. Find all closest clusters, accumulate sums of points belonging to clusters for new center computation
        for (u32 i = 0; i < colorCount; ++i) {
            argb32 color = colorOf(i);
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
            if (acc.count == 0) {
                // Degenerate situation where a cluster center has no points closest to it.
                // Not even sure if this case can ever occur; it didn't ever happen in my tests.
                // We handle it anyways to make sure.
                VXIO_DEBUG_ASSERT_EQ(detail::lengthSqr(acc.sum.cast<int>()), 0);
                VXIO_LOG(WARNING, "Rare degenerate case in k-means clustering occured (isolated cluster center)");
                newCenters.insert(pack4b(acc.previousCenter), clusterIndex);
                continue;
            }

            const Vec<u32, 4> center = acc.sum / acc.count;
            const Vec4u8 center8 = center.cast<u8>();
            if constexpr (build::DEBUG) {
                for (usize i = 0; i < 4; ++i) {
                    VXIO_DEBUG_ASSERT_LT(center[i], 256);
                }
            }

            newCenters.insert(pack4b(center8), clusterIndex);
            anyChange |= center8 != acc.previousCenter;
            totalChange += detail::distanceSqr(center8, acc.previousCenter);
            acc = {center8, {}, 0};
        }
        clusterCenters = std::move(newCenters);

        // 3. Repeat until convergence
        VXIO_LOG(DETAIL, "Iteration " + stringify(iterations) + ": total distance change = " + stringify(totalChange));
    } while (anyChange);

    VXIO_LOG(DETAIL, "Performed k-means clustering in " + stringify(iterations) + " iterations");

    HexTree points;
    for (u32 i = 0; i < colorCount; ++i) {
        points.insert(colorOf(i), i);
    }

    for (u32 i = 0; i < colorCount; ++i) {
        argb32 color = colorOf(i);
        Vec4u8 point = unpack4b(color);
        std::pair<Vec4u8, u32> closestClusterCenter = clusterCenters.closest(point);
        // TODO consider caching the representative for each center instead of computing it here
        std::pair<Vec4u8, u32> representativeColor = points.closest(closestClusterCenter.first);

        result[i] = representativeColor.second;
    }

    return result;
}

}  // namespace voxelio
