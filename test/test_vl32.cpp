#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"
#include "voxelio/test/voxels.hpp"

#include "voxelio/format/vl32.hpp"
#include "voxelio/voxelarray.hpp"

namespace voxelio::test {

VXIO_TEST(vl32, readDebugModel)
{
    FileInputStream stream = openTestAsset("debug.vl32");

    voxelio::vl32::Reader reader{stream};
    voxelio::VoxelArray voxels{8, 8, 8};
    readIteratively(reader, voxels);

    verifyDebugModelVoxels(voxels);
}

VXIO_TEST(vl32, writeAndReadEqual)
{
    constexpr size_t dims = 10;
    voxelio::VoxelArray voxels = makeRandomVoxels(dims, dims, dims, true);

    const std::string path = suggestRandomOutputPath();
    {
        FileOutputStream stream = openForWriteOrFail(path);
        voxelio::vl32::Writer writer(stream);
        /*size_t writeCount = */ writeIteratively(writer, voxels);
        VXIO_ASSERT(stream.good());
        // VXIO_ASSERT_EQ(writeCount, voxels.countVoxels());
    }

    voxelio::VoxelArray actualVoxels{dims, dims, dims};
    {
        FileInputStream stream = openForReadOrFail(path);
        voxelio::vl32::Reader reader(stream);
        VXIO_ASSERT(stream.good());
        readIteratively(reader, actualVoxels);
        VXIO_ASSERT(not stream.err());
    }

    // VXIO_ASSERT_EQ(actualVoxels.countVoxels(), voxels.countVoxels());
    VXIO_ASSERT_EQ(actualVoxels, voxels);
}

}  // namespace voxelio::test
