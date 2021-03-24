#ifndef VXIO_TEST_VOXELS_HPP
#define VXIO_TEST_VOXELS_HPP

#include "voxelio/test/random.hpp"

#include "voxelio/palette.hpp"
#include "voxelio/voxelarray.hpp"

namespace voxelio::test {

void verifyDebugModelVoxels(const VoxelArray &voxels);

void writeRandomVoxels(VoxelArray &out, u32 seed);

Palette32 paletteFromVoxels(const VoxelArray &voxels);

inline VoxelArray makeRandomVoxels(usize x, usize y, usize z, u32 seed = DEFAULT_SEED)
{
    VoxelArray result{x, y, z};
    writeRandomVoxels(result, seed);
    return result;
}

}  // namespace voxelio::test

#endif  // VOXELS_HPP
