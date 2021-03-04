#ifndef VXIO_PALETTE_HPP
#define VXIO_PALETTE_HPP

#include "assert.hpp"
#include "primitives.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace voxelio {

/**
 * @brief Assists in the creation of palettes from ARGB colors.
 */
class Palette32 {
private:
    std::unordered_map<argb32, u32> colorToIndex;
    std::vector<argb32> indexToColor;

public:
    Palette32() = default;
    Palette32(const Palette32 &) = default;
    Palette32(Palette32 &&) = default;

    Palette32 &operator=(const Palette32 &) = default;
    Palette32 &operator=(Palette32 &&) = default;

    bool empty() const
    {
        return colorToIndex.empty();
    }

    usize size() const
    {
        return indexToColor.size();
    }

    const argb32 *data() const
    {
        return indexToColor.data();
    }

    /**
     * @brief Returns the color at the given palette index.
     * This method fails if the index is >= the palette size.
     * @param index the palette index
     * @return the color
     */
    argb32 colorOf(u32 index) const
    {
        VXIO_DEBUG_ASSERT_LT(index, indexToColor.size());
        return indexToColor[index];
    }

    /**
     * @brief Returns the palette index of the given color.
     * This method fails if the color is not in the palette.
     * @param color the color
     * @return the palette index
     */
    u32 indexOf(argb32 color) const
    {
        auto iter = colorToIndex.find(color);
        VXIO_DEBUG_ASSERTM(iter != colorToIndex.end(), "Color 0x" + stringifyHex(color) + " not found in palette");
        return iter->second;
    }

    /**
     * @brief Inserts a new color into the palette.
     * @param color the argb color to insert
     * @return the index of the new color or the index of an already inserted, identical color
     */
    u32 insert(argb32 color)
    {
        auto [location, success] = colorToIndex.emplace(color, size());
        if (success) {
            indexToColor.push_back(color);
        }
        VXIO_DEBUG_ASSERT_EQ(indexToColor.size(), colorToIndex.size());
        return location->second;
    }

    /**
     * @brief Clears the palette.
     * This removes all elements but doesn't necessarily free the memory claimed by this data structure.
     */
    void clear()
    {
        colorToIndex.clear();
        indexToColor.clear();
    }

    /**
     * @brief Reserves memory for a given amount of elements.
     * @param capacity the desired capacity in elements
     */
    void reserve(usize capacity)
    {
        colorToIndex.reserve(capacity);
        indexToColor.reserve(capacity);
    }

    /**
     * @brief Finds a reduction in size for this palette.
     * @param desiredSize the desired size of the output palette
     * @return a table mapping from palette indices in this palette to representative indices
     */
    std::unique_ptr<u32[]> reduce(usize desiredSize, usize &outSize) const;
};

}  // namespace voxelio

#endif  // VXIO_PALETTE_HPP
