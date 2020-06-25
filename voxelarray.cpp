#include "voxelarray.hpp"

namespace voxelio {

std::unique_ptr<Color32[]> VoxelArray::copyContents(Color32 source[], size_t size)
{
    auto result = std::make_unique<Color32[]>(size);
    std::copy_n(source, size, result.get());
    return result;
}

VoxelArray::VoxelArray(std::unique_ptr<Color32[]> voxels, size_t x, size_t y, size_t z)
    : size{x, y, z}, _sizeXY{x * y}, _volume{x * y * z}, voxels{std::move(voxels)}
{
}

// can't use make_unique_default_init prior to C++20
VoxelArray::VoxelArray(size_t x, size_t y, size_t z)
    : VoxelArray{(x | y | z) == 0 ? nullptr : std::unique_ptr<Color32[]>{new Color32[x * y * z]{}}, x, y, z}
{
    VXIO_DEBUG_ASSERT_CONSEQUENCE((x * y * z) == 0, (x | y | z) == 0);
}

size_t VoxelArray::countVoxels() const
{
    size_t result = 0;
    for (size_t i = 0; i < volume(); ++i) {
        result += voxels[i].isVisible();
    }
    return result;
}

std::string VoxelArray::toString() const
{
    std::string result = "VoxelArray{dims=";
    result += stringify(size.x()) + "x" + stringify(size.y()) + "x" + stringify(size.z());
    result += ", volume=" + stringify(volume());
    result += '}';
    return result;
}

bool VoxelArray::operator==(const VoxelArray &other) const
{
    if (this->size != other.size) {
        return false;
    }
    size_t lim = this->volume();
    for (size_t i = 0; i < lim; ++i) {
        Color32 v0 = this->voxels[i];
        Color32 v1 = other.voxels[i];
        if ((v0 | v1).isVisible() && v0 != v1) {
            return false;
        }
    }
    return true;
}

void VoxelArray::fill(Color32 color)
{
    for (size_t i = 0; i < _volume; ++i) {
        voxels[i] = color;
    }
}

void VoxelArray::forEachPosition(const std::function<void(Vec3size)> &action) const
{
    for (size_t z = 0; z < size.z(); z++)
        for (size_t y = 0; y < size.y(); y++)
            for (size_t x = 0; x < size.x(); x++)
                action({x, y, z});
}

}  // namespace voxelio
