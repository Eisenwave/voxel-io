#include "palette.hpp"

#include "assert.hpp"

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

}  // namespace voxelio
