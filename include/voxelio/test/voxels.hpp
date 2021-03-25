#ifndef VXIO_TEST_VOXELS_HPP
#define VXIO_TEST_VOXELS_HPP

#include "voxelio/test/random.hpp"

#include "voxelio/palette.hpp"
#include "voxelio/voxelarray.hpp"

namespace voxelio::test {

void verifyDebugModelVoxels(const VoxelArray &voxels);

void writeRandomVoxels(VoxelArray &out, bool transparency, u32 seed = DEFAULT_SEED);

Palette32 paletteFromVoxels(const VoxelArray &voxels);

inline VoxelArray makeRandomVoxels(usize x, usize y, usize z, bool transparency, u32 seed = DEFAULT_SEED)
{
    VoxelArray result{x, y, z};
    writeRandomVoxels(result, transparency, seed);
    return result;
}

}  // namespace voxelio::test

#endif  // VOXELS_HPP
