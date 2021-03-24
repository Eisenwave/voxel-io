#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"
#include "voxelio/test/voxels.hpp"

#include "voxelio/color.hpp"
#include "voxelio/format/qb.hpp"
#include "voxelio/format/vox.hpp"
#include "voxelio/log.hpp"
#include "voxelio/stream.hpp"
#include "voxelio/types.hpp"

namespace voxelio::test {
namespace {

constexpr Color32 hsbToRgb(float hue, float saturation, float brightness) noexcept
{
    u8 r = 0, g = 0, b = 0;
    if (saturation < (1 / 255.f)) {
        r = g = b = static_cast<u8>(brightness * 255.0f + 0.5f);
        return {r, g, b};
    }

    float h = (hue - std::floor(hue)) * 6.0f;
    float f = h - std::floor(h);

    float p = brightness * (1.0f - saturation);
    float q = brightness * (1.0f - saturation * f);
    float t = brightness * (1.0f - saturation * (1.0f - f));

    switch (static_cast<unsigned>(h)) {
    case 0:
        r = static_cast<u8>(brightness * 255.0f + 0.5f);
        g = static_cast<u8>(t * 255.0f + 0.5f);
        b = static_cast<u8>(p * 255.0f + 0.5f);
        break;
    case 1:
        r = static_cast<u8>(q * 255.0f + 0.5f);
        g = static_cast<u8>(brightness * 255.0f + 0.5f);
        b = static_cast<u8>(p * 255.0f + 0.5f);
        break;
    case 2:
        r = static_cast<u8>(p * 255.0f + 0.5f);
        g = static_cast<u8>(brightness * 255.0f + 0.5f);
        b = static_cast<u8>(t * 255.0f + 0.5f);
        break;
    case 3:
        r = static_cast<u8>(p * 255.0f + 0.5f);
        g = static_cast<u8>(q * 255.0f + 0.5f);
        b = static_cast<u8>(brightness * 255.0f + 0.5f);
        break;
    case 4:
        r = static_cast<u8>(t * 255.0f + 0.5f);
        g = static_cast<u8>(p * 255.0f + 0.5f);
        b = static_cast<u8>(brightness * 255.0f + 0.5f);
        break;
    case 5:
        r = static_cast<u8>(brightness * 255.0f + 0.5f);
        g = static_cast<u8>(p * 255.0f + 0.5f);
        b = static_cast<u8>(q * 255.0f + 0.5f);
        break;
    }
    return {r, g, b};
}

#define VXIO_VEC_3I8(x, y, z) Vec3i8(i8{x}, i8{y}, i8{z})

#define VXIO_MATRIX_I8(a, b, c, d, e, f, g, h, i)                           \
    std::array<Vec3i8, 3>                                                   \
    {                                                                       \
        VXIO_VEC_3I8(a, b, c), VXIO_VEC_3I8(d, e, f), VXIO_VEC_3I8(g, h, i) \
    }

constexpr bool DUMP_TEST_FILE = false;

VXIO_TEST(vox, debugModelReadCorrectly)
{
    FileInputStream stream = openTestAsset("debug.vox", OpenMode::BINARY);

    vox::Reader reader{stream};
    reader.setFixGravity(true);

    VoxelArray debugModelVoxels{8, 8, 8};
    readIterativelyAndMoveToOrigin(reader, debugModelVoxels);
    verifyDebugModelVoxels(debugModelVoxels);
}

VXIO_TEST(vox, transformationConcatenation_manual)
{
    static constexpr auto MATRIX_A = VXIO_MATRIX_I8(1, 5, 7, -9, -23, 6, -3, 5, 3);
    static constexpr auto MATRIX_B = VXIO_MATRIX_I8(9, 3, 0, 1, 1, 1, -2, -4, -8);
    static constexpr auto MATRIX_AB = VXIO_MATRIX_I8(0, -20, -51, -116, -74, -71, -28, -16, -19);
    static constexpr auto MATRIX_BA = VXIO_MATRIX_I8(-18, -24, 81, -11, -13, 16, 58, 42, -62);

    auto a = vox::Transformation{MATRIX_A, {0, 0, 0}};
    auto b = vox::Transformation{MATRIX_B, {0, 0, 0}};
    auto ab = vox::Transformation::concat(a, b);
    auto ba = vox::Transformation::concat(b, a);

    VXIO_ASSERT(ab.matrix == MATRIX_AB);
    VXIO_ASSERT(ba.matrix == MATRIX_BA);
}

VXIO_TEST(vox, writeTestStick)
{
    constexpr size_t voxelCount = 1024;

    ByteArrayOutputStream outStream;
    {
        vox::Writer writer{outStream};

        size_t index = 0;
        while (writer.palette().size() < voxelCount) {
            float hue = static_cast<float>(index++) / voxelCount;
            Color32 color = hsbToRgb(hue, 1, 1);
            writer.palette().insert(color);
        }

        ResultCode initResult = writer.init();
        VXIO_ASSERT(isGood(initResult));

        for (uint32_t i = 0; i < voxelCount; ++i) {
            Vec3i32 pos{static_cast<int32_t>(i), 0, 0};
            Voxel32 voxel{pos, {i}};
            ResultCode result = writer.write(&voxel, 1);
            VXIO_ASSERT(isGood(result));
        }

        VXIO_ASSERT(isGood(writer.finalize()));
    }

    if (DUMP_TEST_FILE) {
        VXIO_LOG(DEBUG, "Dumping test VOX file to /tmp/test_stick.vox");
        FileOutputStream stream = openForWriteOrFail(getOutputPath() + '/' + "test_stick.vox");
        stream.write(outStream.data(), outStream.size());
    }
}

VXIO_TEST(vox, writeAndReadMatch)
{
    constexpr size_t voxelCount = 255;

    default_rng rng{12345};
    std::uniform_int_distribution<int32_t> distr{-1024, 1023};
    std::uniform_int_distribution<uint32_t> colorDistr;

    std::unordered_map<Vec3i32, uint32_t> voxels;

    ByteArrayOutputStream outStream;
    {
        vox::Writer writer{outStream};

        while (writer.palette().size() < voxelCount) {
            writer.palette().insert(colorDistr(rng));
        }

        ResultCode initResult = writer.init();
        VXIO_ASSERT(isGood(initResult));

        for (uint32_t i = 0; i < voxelCount; ++i) {
            Vec3i32 pos{distr(rng), distr(rng), distr(rng)};
            Voxel32 voxel{pos, {i}};
            ResultCode result = writer.write(&voxel, 1);
            VXIO_ASSERT(isGood(result));

            voxels.insert({pos, writer.palette().colorOf(i)});
        }

        VXIO_ASSERT(isGood(writer.finalize()));
    }

    ByteArrayInputStream inStream{outStream};
    {
        vox::Reader reader{inStream};
        auto initResult = reader.init();
        VXIO_ASSERT(isGood(initResult));

        Voxel32 buffer;
        for (size_t i = 0; i < voxelCount; ++i) {
            VXIO_ASSERT(isGood(reader.read(&buffer, 1)));

            auto location = voxels.find(buffer.pos);
            VXIO_ASSERT(location != voxels.end());
            VXIO_ASSERT_EQ(buffer.argb, location->second);
        }
    }
}

}  // namespace
}  // namespace voxelio::test
