#ifndef VXIO_PALETTE_HPP
#define VXIO_PALETTE_HPP

#include "primitives.hpp"

#include <memory>
#include <unordered_map>

namespace voxelio {

/**
 * @brief Assists in the creation of palettes from ARGB colors.
 */
class Palette32 {
private:
    std::unordered_map<argb32, u32> colorToIndexMap;

public:
    Palette32() = default;
    Palette32(const Palette32 &) = default;
    Palette32(Palette32 &&) = default;

    Palette32 &operator=(const Palette32 &) = default;
    Palette32 &operator=(Palette32 &&) = default;

    std::unique_ptr<argb32[]> build() const;

    void clear()
    {
        colorToIndexMap.clear();
    }

    bool empty() const
    {
        return colorToIndexMap.empty();
    }

    u32 indexOf(argb32 color) const
    {
        return colorToIndexMap.at(color);
    }

    /**
     * @brief Inserts a new color into the palette.
     * @param color the argb color to insert
     * @return the index of the new color or the index of an already inserted, identical color
     */
    u32 insert(argb32 color);

    /**
     * @brief Inserts a new color into the palette. This method skips duplicate-checks and assumes that the added
     * color is guaranteed to be unique.
     * @param color the argb color to insert
     * @return the index of the new color
     */
    u32 insertUnsafe(argb32 color);

    void reserve(usize capacity)
    {
        colorToIndexMap.reserve(capacity);
    }

    usize size() const
    {
        return colorToIndexMap.size();
    }
};

}  // namespace voxelio

#endif  // VXIO_PALETTE_HPP
