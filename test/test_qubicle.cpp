#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"
#include "voxelio/test/voxels.hpp"

#include "voxelio/format/qb.hpp"
#include "voxelio/format/qef.hpp"
#include "voxelio/parse.hpp"
#include "voxelio/stringify.hpp"

namespace voxelio::test {
namespace {

constexpr usize TEST_VOXELS_SIZE = 16;

void qef_iterativeWriteAndReadEqual_write(ByteArrayOutputStream &stream)
{
    FileOutputStream tempStream = openForWriteOrFail(getOutputPath() + '/' + "qef_iterativeWriteAndReadEqual.qef");

    for (auto *stream : std::initializer_list<OutputStream *>{&stream, &tempStream}) {
        const VoxelArray &voxels = makeRandomVoxels(TEST_VOXELS_SIZE, TEST_VOXELS_SIZE, TEST_VOXELS_SIZE);

        qef::Writer writer{*stream};
        ResultCode sizeResult = writer.setGlobalVolumeSize(voxels.dimensions().cast<uint32_t>());
        VXIO_ASSERT(isGood(sizeResult));
        writer.palette() = paletteFromVoxels(voxels);

        VXIO_ASSERT(not writer.palette().empty());
        VXIO_ASSERT(isGood(writer.init()));

        VXIO_ASSERT(stream->good());

        test::writeIterativelyUsingPalette(writer, voxels);
        VXIO_ASSERT(stream->good());
    }

    VXIO_ASSERT(stream.good());
}

void qef_iterativeWriteAndReadEqual_read(uint8_t buffer[], size_t bufferSize)
{
    VoxelArray voxels = makeRandomVoxels(TEST_VOXELS_SIZE, TEST_VOXELS_SIZE, TEST_VOXELS_SIZE);

    ByteArrayInputStream stream{buffer, bufferSize};
    qef::Reader reader{stream};
    VXIO_ASSERT(stream.good());

    ReadResult initResult = reader.init();
    VXIO_ASSERT(initResult.isGood());
    VXIO_ASSERT(stream.good());

    readIteratively(reader, voxels);

    VXIO_ASSERT(stream.eof());
    VXIO_ASSERT(not stream.err());

    verifyDebugModelVoxels(voxels);
}

void verifyDebugModel(const qb::Model &model)
{
    VXIO_ASSERT_EQ(model.matrixCount(), size_t{1});
    const qb::Matrix &onlyMatrix = model.matrices().front();
    VXIO_ASSERT_EQ(onlyMatrix.size, Vec3size(8, 8, 8));
    verifyDebugModelVoxels(onlyMatrix.voxels);
}

VXIO_TEST(qb, readDebugModelCorrectly)
{
    FileInputStream stream = openTestAsset("debug.qb", OpenMode::BINARY);

    qb::Model debugModel = qb::Deserializer{stream}.read();
    verifyDebugModel(debugModel);
}

VXIO_TEST(qef, readDebugModelCorrectly)
{
    FileInputStream stream = openTestAsset("debug.qef", OpenMode::BINARY);

    const VoxelArray debugModelVoxels = qef::Deserializer{stream}.read();
    verifyDebugModelVoxels(debugModelVoxels);
}

VXIO_TEST(qb, serializeAndDeserializeEqual)
{
    const VoxelArray voxels = makeRandomVoxels(TEST_VOXELS_SIZE, TEST_VOXELS_SIZE, TEST_VOXELS_SIZE);
    const qb::Model outModel{qb::Matrix{"test", {}, voxels}};

    ByteArrayOutputStream outStream;
    qb::Serializer{outStream}.write(outModel);
    VXIO_ASSERT(outStream.good());

    ByteArrayInputStream inStream{outStream.data(), outStream.size()};
    const qb::Model inModel = qb::Deserializer{inStream}.read();
    VXIO_ASSERT(inStream.good());
    VXIO_ASSERT_EQ(inModel, outModel);
}

VXIO_TEST(qef, isColorPrintingLossless)
{
    constexpr unsigned lim = 256;
    constexpr unsigned precision = 3;

    for (unsigned i = 0; i < lim; ++i) {
        // TODO create qef::stringifyRgb or something instead of imitating the behavior of internal code
        std::string str = stringifyFraction(i, lim - 1, precision);
        float f;
        bool parseSuccess = parse(str, f);
        VXIO_ASSERT(parseSuccess);
        if (parseSuccess) {
            VXIO_ASSERT_EQ(std::lround(f * (lim - 1)), i);
        }
    }
}

VXIO_TEST(qef, iterativeWriteAndReadEqual)
{
    ByteArrayOutputStream outStream;

    qef_iterativeWriteAndReadEqual_write(outStream);
    qef_iterativeWriteAndReadEqual_read(outStream.data(), outStream.size());
}

}  // namespace
}  // namespace voxelio::test
