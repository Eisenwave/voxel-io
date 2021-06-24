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
using detail::dileave4b;
using detail::ileave4b;
using detail::pack4b;
using detail::unpack4b;

struct MortonAndIndex {
    u32 morton;
    u32 index;
};

// K-MEANS IMPLEMENTATION ==============================================================================================

namespace kmeans {

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
            const u32 distanceToOtherClusters = clusterCenters.distanceSqr(randomColor);
            avgDistance += distanceToOtherClusters;
            VXIO_DEBUG_ASSERT_NE(distanceToOtherClusters, 0u);

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

// 1. Find all closest clusters, accumulate sums of points belonging to clusters for new center computation
void accumulatePointsToClusters(const argb32 colors[],
                                usize colorCount,
                                const HexTree &clusterCenters,
                                Accumulator accumulators[],
                                [[maybe_unused]] usize clusterCount)
{
    for (u32 i = 0; i < colorCount; ++i) {
        argb32 color = colors[i];
        std::pair<argb32, u32> closest = clusterCenters.closest(color);
        VXIO_DEBUG_ASSERT(clusterCenters.contains(closest.first));
        VXIO_DEBUG_ASSERT_LT(closest.second, clusterCount);
        Accumulator &acc = accumulators[closest.second];
        acc.sum += unpack4b(color).cast<u32>();
        acc.count += 1;
    }
}

// 2. Compute new cluster centers and reset accumulators
HexTree computeNewClusterCentersAndResetAccumulators(Accumulator accumulators[],
                                                     usize clusterCount,
                                                     u64 &outTotalChange)
{
    outTotalChange = 0;

    HexTree newCenters;
    for (u32 clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
        Accumulator &acc = accumulators[clusterIndex];
        if (acc.count == 0) {
            // Degenerate situation where a cluster center has no points closest to it.
            // Not even sure if this case can ever occur; it didn't ever happen in my tests.
            // We handle it anyways to make sure.
            VXIO_DEBUG_ASSERT_EQ(detail::lengthSqr(acc.sum.cast<int>()), 0u);
            VXIO_LOG(WARNING, "Rare degenerate case in k-means clustering occured (isolated cluster center)");
            newCenters.insert(pack4b(acc.previousCenter), clusterIndex);
            continue;
        }

        const Vec<u32, 4> center = acc.sum / acc.count;
        const Vec4u8 center8 = center.cast<u8>();
        if constexpr (build::DEBUG) {
            for (usize i = 0; i < 4; ++i) {
                VXIO_DEBUG_ASSERT_LT(center[i], 256u);
            }
        }

        newCenters.insert(pack4b(center8), clusterIndex);
        outTotalChange += detail::distanceSqr(center8, acc.previousCenter);
        acc = {center8, {}, 0};
    }

    return newCenters;
}

}  // namespace kmeans

// TRUNCATION IMPLEMENTATION ===========================================================================================

namespace truncate {

u32 chooseRepresentative(const MortonAndIndex pairs[], const u32 count)
{
    VXIO_DEBUG_ASSERT_NE(count, 0u);

    // the highest morton digit / nibble should is assumed to be the same for all values
    const u8 highestNibble = pairs[0].morton >> 28;
    const u8 highestByte = highestNibble | u8(highestNibble << 4);

    const u32 hypercubeCornerMorton = pack4b({highestByte, highestByte, highestByte, highestByte});
    const Vec4u8 origin = unpack4b(dileave4b(hypercubeCornerMorton));

    u32 closestIndex = 0;
    u32 closestDistance = ~u32{0};

    for (u32 i = 0; i < count; ++i) {
        const u32 morton = pairs[i].morton;
        const argb32 color = dileave4b(morton);
        const Vec4u8 point = unpack4b(color);
        const u32 distance = detail::distanceSqr(origin, point);
        if (distance < closestDistance) {
            closestDistance = distance;
            closestIndex = pairs->index;
        }
    }

    return closestIndex;
}

constexpr usize BITS = 2;
constexpr usize SHIFT = BITS * 4;
constexpr usize MAX_NEIGHBORS = 1u << SHIFT;

/// Stores the representative indices of the current buffer of neighbors.
/// If there are more than 1 representative neighbor in the buffer, a single representative is chosen.
void storeRepresentative(u32 result[], const MortonAndIndex buffer[], const u32 neighborCount)
{
    VXIO_DEBUG_ASSERT_LT(neighborCount, MAX_NEIGHBORS);
    VXIO_ASSUME(neighborCount < MAX_NEIGHBORS);

    u32 representativeIndex = buffer[0].index;
    if (neighborCount > 1) {
        VXIO_LOG(SPAM, "Found block of " + stringify(neighborCount) + " neighbors");
        representativeIndex = chooseRepresentative(buffer, neighborCount);
    }
    for (u32 i = 0; i < neighborCount; ++i) {
        result[buffer[i].index] = representativeIndex;
    }
}

}  // namespace truncate

// OTHER UTILITY =======================================================================================================

std::unique_ptr<u32[]> identityReduction(usize colorCount)
{
    auto result = std::make_unique<u32[]>(colorCount);
    for (u32 i = 0; i < colorCount; ++i) {
        result[i] = i;
    }
    return result;
}

template <typename T>
Palette32 createReducedPalette_impl(const Palette32 &palette, T inOutReduction[])
{
    Palette32 result;
    const usize colorCount = palette.size();
    for (u32 i = 0; i < colorCount; ++i) {
        const u32 representativeIndex = inOutReduction[i];
        [[maybe_unused]] const u32 resultIndex = result.insert(palette.colorOf(representativeIndex));

        if constexpr (not std::is_const_v<T>) {
            inOutReduction[i] = resultIndex;
        }
    }
    return result;
}

}  // namespace

// PALETTE32 MEMBER FUNCTIONS ==========================================================================================

Palette32 Palette32::createReducedPalette(const u32 reduction[]) const
{
    return createReducedPalette_impl(*this, reduction);
}

Palette32 Palette32::createReducedPaletteAndStoreMapping(u32 inOutReduction[]) const
{
    return createReducedPalette_impl(*this, inOutReduction);
}

std::unique_ptr<u32[]> Palette32::reduce(usize desiredSize, usize &outSize) const
{
    const usize colorCount = size();

    outSize = std::min(colorCount, desiredSize);

    // Return identity mapping if no reduction is necessary.
    if (colorCount == outSize) {
        VXIO_LOG(DETAIL, "No reduction for palette size " + stringify(desiredSize) + " necessary");
        return identityReduction(colorCount);
    }

    usize truncationSize;
    auto truncation = reduceByTruncating(desiredSize, truncationSize);
    VXIO_ASSERT_NOTNULL(truncation);

    if (truncationSize == desiredSize) {
        VXIO_LOG(DETAIL,
                 "Completed reduction process " + stringify(colorCount) + " -> " + stringify(desiredSize) +
                     " with only truncation");
        return truncation;
    }
    else if (truncationSize == colorCount) {
        VXIO_LOG(DETAIL,
                 "No colors truncated, reducing by clustering " + stringify(colorCount) + " -> " + stringify(outSize));
        return reduceByClustering(desiredSize);
    }

    VXIO_LOG(DETAIL, "Truncation reduced colors from " + stringify(colorCount) + " -> " + stringify(truncationSize));
    VXIO_LOG(DETAIL, "Reducing truncated from " + stringify(truncationSize) + " -> " + stringify(outSize));

    const Palette32 truncated = createReducedPaletteAndStoreMapping(truncation.get());
    VXIO_ASSERT_EQ(truncated.size(), truncationSize);

    std::unique_ptr<u32[]> truncatedReduction = truncated.reduceByClustering(desiredSize);
    std::unique_ptr<u32[]> result = std::make_unique<u32[]>(colorCount);

    // the contents of truncation have been modified, so it is now a mapping from our indices onto truncated indices
    for (u32 i = 0; i < colorCount; ++i) {
        const u32 truncatedIndex = truncation[i];
        const u32 truncatedRepresentative = truncatedReduction[truncatedIndex];

        VXIO_DEBUG_ASSERT_LT(truncatedIndex, truncationSize);
        VXIO_DEBUG_ASSERT_LT(truncatedRepresentative, truncationSize);

        const argb32 truncatedColor = truncated.colorOf(truncatedRepresentative);
        result[i] = indexOf(truncatedColor);
    }

    return result;
}

std::unique_ptr<u32[]> Palette32::reduceByTruncating(const usize minimumSize, usize &outSize) const
{
    VXIO_ASSERT_GT(size(), minimumSize);

    HexTree colorTree;
    const usize colorCount = size();
    for (u32 i = 0; i < colorCount; ++i) {
        colorTree.insert(colorOf(i), i);
    }
    auto result = std::make_unique<u32[]>(size());

    MortonAndIndex neighborBuffer[truncate::MAX_NEIGHBORS]{};
    u32 neighborCount = 0;
    u32 previous = 1;
    u32 allowedReductions = static_cast<u32>(size() - minimumSize);

    colorTree.forEachMorton([&](const u32 morton, const u32 index) -> void {
        const u32 mortonTruncated = morton & (~u32{0} << truncate::SHIFT);
        if (mortonTruncated == previous && neighborCount <= allowedReductions) {
            VXIO_DEBUG_ASSERT_LT(neighborCount, truncate::MAX_NEIGHBORS);
            neighborBuffer[neighborCount++] = {morton, index};
            return;
        }

        VXIO_DEBUG_ASSERT_CONSEQUENCE(allowedReductions == 0, neighborCount < 2);
        truncate::storeRepresentative(result.get(), neighborBuffer, neighborCount);

        const u32 changeOfAllowed = (neighborCount != 0) * (neighborCount - 1);
        VXIO_DEBUG_ASSERT_CONSEQUENCE(allowedReductions == 0, changeOfAllowed == 0);
        allowedReductions -= changeOfAllowed;

        neighborCount = 1;
        neighborBuffer[0] = {morton, index};
        previous = mortonTruncated;
    });

    // We need one last call for the remaining neighbors in the buffer
    truncate::storeRepresentative(result.get(), neighborBuffer, neighborCount);

    const u32 finalChangeOfAllowed = (neighborCount != 0) * (neighborCount - 1);
    VXIO_DEBUG_ASSERT_CONSEQUENCE(allowedReductions == 0, finalChangeOfAllowed == 0);
    allowedReductions -= finalChangeOfAllowed;

    outSize = allowedReductions + minimumSize;
    VXIO_ASSERT_LE(outSize, size());
    return result;
}

std::unique_ptr<u32[]> Palette32::reduceByClustering(const usize desiredSize) const
{
    VXIO_ASSERT_GT(size(), desiredSize);

    const usize clusterCount = desiredSize;
    const usize colorCount = size();

    auto result = std::make_unique<u32[]>(colorCount);

    /// Maps from cluster center points to a unique index of the center.
    HexTree clusterCenters = kmeans::seedClusterCenters(data(), colorCount, clusterCount);

    auto clusterAccumulators = std::make_unique<kmeans::Accumulator[]>(clusterCount);
    clusterCenters.forEach([&clusterAccumulators](argb32 center, u32 index) -> void {
        clusterAccumulators[index].previousCenter = unpack4b(center);
    });

    usize iterations = 0;
    u64 totalChange;
    do {
        ++iterations;

        accumulatePointsToClusters(data(), size(), clusterCenters, clusterAccumulators.get(), clusterCount);
        clusterCenters =
            computeNewClusterCentersAndResetAccumulators(clusterAccumulators.get(), clusterCount, totalChange);
        VXIO_LOG(DETAIL, "Iteration " + stringify(iterations) + ": total distance change = " + stringify(totalChange));

    } while (totalChange != 0);

    VXIO_LOG(DETAIL, "Performed k-means clustering in " + stringify(iterations) + " iterations");

    HexTree points;
    for (u32 i = 0; i < colorCount; ++i) {
        points.insert(colorOf(i), i);
    }

    for (u32 i = 0; i < colorCount; ++i) {
        argb32 color = colorOf(i);
        std::pair<argb32, u32> closestClusterCenter = clusterCenters.closest(color);
        // TODO consider caching the representative for each center instead of computing it here
        std::pair<argb32, u32> representativeColor = points.closest(closestClusterCenter.first);

        result[i] = representativeColor.second;
    }

    return result;
}

}  // namespace voxelio
