#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/deflate.hpp"

namespace voxelio::test {
namespace {

VXIO_TEST(deflate, deflateWorksWithoutErrors)
{
    constexpr size_t bufferSize = 1024;
    constexpr unsigned compressionLevel = 1;

    uint8_t buffer[bufferSize];

    FileInputStream stream = openTestAsset("waifu.jpg", OpenMode::BINARY);

    FileOutputStream tempStreamRaw = openForWriteOrFail(getOutputPath() + '/' + "waifu.jpg", OpenMode::BINARY);
    FileOutputStream tempStreamDef = openForWriteOrFail(getOutputPath() + '/' + "waifu.jpg.zlib", OpenMode::BINARY);
    deflate::Deflator deflator{tempStreamDef, compressionLevel};

    while (not stream.eof()) {
        size_t read = stream.read(buffer, bufferSize);

        tempStreamRaw.write(buffer, read);
        VXIO_ASSERT(not tempStreamRaw.err());

        deflate::ResultCode deflResult = deflator.deflate(buffer, read);
        VXIO_ASSERT_EQ(deflResult, deflate::ResultCode::OK);
    }
    deflate::ResultCode flushResult = deflator.flush();
    VXIO_ASSERT_EQ(flushResult, deflate::ResultCode::OK);
}

static constexpr size_t INFLATE_OUT_SIZE = 1024 * 1024 * 16;
static uint8_t INFLATE_OUT_BUFFER[INFLATE_OUT_SIZE];

VXIO_TEST(deflate, inflateUsingMinizWorksWithoutErrors)
{
    const std::string fullPath = ASSET_PATH + std::string{"/waifu.jpg.zlib"};
    const usize expectedFileSize = getFileSize(fullPath);

    FileInputStream stream = openTestAsset("waifu.jpg.zlib", OpenMode::BINARY);

    ByteArrayOutputStream deflOutput;
    pipeOrFail(stream, deflOutput);
    VXIO_ASSERT_EQ(deflOutput.size(), expectedFileSize);

    mz_ulong size = INFLATE_OUT_SIZE;
    deflate::ResultCode result{mz_uncompress(INFLATE_OUT_BUFFER, &size, deflOutput.data(), deflOutput.size())};
    VXIO_ASSERT_EQ(result, deflate::ResultCode::OK);
}

VXIO_TEST(deflate, inflateUsingInflatorWorksWithoutErrors)
{
    constexpr bool dumpInflData = true;
    constexpr size_t bufferSize = 1024;

    uint8_t buffer[bufferSize];

    FileInputStream stream = openTestAsset("waifu.jpg.zlib", OpenMode::BINARY);
    ByteArrayOutputStream rawOutput;
    deflate::Inflator inflator{stream};

    do {
        size_t written;
        deflate::ResultCode result{inflator.inflate(buffer, bufferSize, written)};
        VXIO_ASSERT_EQ(result, deflate::ResultCode::OK);

        rawOutput.write(buffer, written);
    } while (not inflator.eof());

    VXIO_ASSERT(stream.eof());

    if constexpr (dumpInflData) {
        FileOutputStream dumpStream = openForWriteOrFail(getOutputPath() + "/waifu_deflated.jpg");
        dumpStream.write(rawOutput.data(), rawOutput.size());
    }
}

VXIO_TEST(deflate, inflateReversesDeflate)
{
    constexpr bool dumpDeflateData = true;
    constexpr bool alternativeDecompressToo = true;

    constexpr size_t bufferSize = 1024;
    constexpr unsigned compressionLevel = 4;

    uint8_t buffer[bufferSize];

    std::string randomData = makeRandomString(1024 * 32, StringType::BYTE, DEFAULT_SEED, 32);

    // DEFLATION

    ByteArrayInputStream rawInput{reinterpret_cast<u8 *>(randomData.data()), randomData.size()};
    ByteArrayOutputStream deflOutput;

    deflate::Deflator deflator{deflOutput, compressionLevel};

    do {
        size_t read = rawInput.read(buffer, bufferSize);
        VXIO_ASSERT(not rawInput.err());

        deflate::ResultCode result = deflator.deflate(buffer, read);
        VXIO_ASSERT_EQ(result, deflate::ResultCode::OK);
    } while (not rawInput.eof());

    deflate::ResultCode flushResult = deflator.flush();
    VXIO_ASSERT_EQ(flushResult, deflate::ResultCode::OK);

    VXIO_ASSERT_EQ(randomData.size(), deflator.totalRead());

    if constexpr (dumpDeflateData) {
        FileOutputStream dumpStream = openForWriteOrFail(getOutputPath() + "/defl_output.zlib");
        dumpStream.write(deflOutput.data(), deflOutput.size());
    }

    // INFLATION

    if constexpr (alternativeDecompressToo) {
        mz_ulong outLen = INFLATE_OUT_SIZE;
        deflate::ResultCode result{mz_uncompress(INFLATE_OUT_BUFFER, &outLen, deflOutput.data(), deflOutput.size())};
        VXIO_ASSERT_EQ(result, deflate::ResultCode::OK);
    }

    ByteArrayOutputStream rawOutput;
    ByteArrayInputStream deflInput{deflOutput};
    deflate::Inflator inflator{deflInput};

    do {
        size_t written;
        deflate::ResultCode result{inflator.inflate(buffer, bufferSize, written)};
        VXIO_ASSERT_EQ(result, deflate::ResultCode::OK);

        rawOutput.write(buffer, written);
    } while (not inflator.eof());

    VXIO_ASSERT_EQ(rawInput.size(), rawOutput.size());

    size_t mismatchOrSize = mismatchIndex(rawInput.data(), rawOutput.data(), rawInput.size());
    VXIO_ASSERT_EQ(mismatchOrSize, rawInput.size());
}

}  // namespace
}  // namespace voxelio::test
