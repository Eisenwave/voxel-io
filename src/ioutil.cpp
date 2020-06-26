#include "ioutil.hpp"

#include "util.hpp"

namespace voxelio {

void VoxelBufferWriteHelper::reset(Voxel32 *buffer, size_t size) noexcept
{
    VXIO_ASSERT_NOTNULL(buffer);
    VXIO_ASSERT_NE(size, 0);

    buffer32 = buffer;
    index = 0;
    limit = size;
    is64 = false;
}

void VoxelBufferWriteHelper::reset(Voxel64 *buffer, size_t size) noexcept
{
    VXIO_ASSERT_NOTNULL(buffer);
    VXIO_ASSERT_NE(size, 0);

    buffer64 = buffer;
    index = 0;
    limit = size;
    is64 = true;
}

void VoxelBufferWriteHelper::emplace(Vec3i32 pos, u32 color) noexcept
{
    VXIO_DEBUG_ASSERT_LT(index, limit);
    if (is64) {
        buffer64[index++] = {static_vec_cast<Vec3i64>(pos), {color}};
    }
    else {
        buffer32[index++] = {pos, {color}};
    }
}

void VoxelBufferWriteHelper::emplace(Vec3i64 pos, u32 color) noexcept
{
    VXIO_DEBUG_ASSERT_LT(index, limit);
    if (is64) {
        buffer64[index++] = {pos, {color}};
    }
    else {
        buffer32[index++] = {static_vec_cast<Vec3i32>(pos), {color}};
    }
}

void VoxelBufferWriteHelper::write(Voxel32 voxel) noexcept
{
    VXIO_DEBUG_ASSERT_LT(index, limit);
    if (is64) {
        buffer64[index++] = voxelCast<Voxel64>(voxel);
    }
    else {
        buffer32[index++] = voxel;
    }
}

void VoxelBufferWriteHelper::write(Voxel64 voxel) noexcept
{
    VXIO_DEBUG_ASSERT_LT(index, limit);
    if (is64) {
        buffer64[index++] = voxel;
    }
    else {
        buffer32[index++] = voxelCast<Voxel32>(voxel);
    }
}

}  // namespace voxelio
