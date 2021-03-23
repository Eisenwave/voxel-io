#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/format/binvox.hpp"
#include "voxelio/types.hpp"

namespace voxelio::test {

VXIO_TEST(binvox, readModelWithoutErrors)
{
    constexpr usize expectedVoxelCount = 1002;

    FileInputStream stream = openTestAsset("chair.binvox", OpenMode::BINARY);

    binvox::Reader reader{stream};

    std::vector<Voxel32> voxels;
    readIteratively(reader, voxels);

    VXIO_ASSERT_EQ(voxels.size(), expectedVoxelCount);
}

}  // namespace voxelio::test
