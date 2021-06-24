#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/stream.hpp"

#include <filesystem>
#include <fstream>

namespace voxelio::test {

VXIO_TEST(stream, writeAndReadRandomFile_FileOutputStream_FileInputStream)
{
    constexpr size_t fileLength = 8192;

    const std::string randomPath = suggestRandomOutputPath();
    auto outStream = openForWriteOrFail(randomPath);

    const std::string contents = makeRandomString(fileLength, StringType::BINARY);
    outStream.writeString(contents);
    outStream.flush();
    VXIO_ASSERT_EQ(outStream.position(), fileLength);
    VXIO_ASSERT(outStream.good());

    {
        std::string actualContents;
        auto inStream = openForReadOrFail(randomPath);
        VXIO_ASSERT(inStream.good());

        inStream.readStringTo(actualContents, fileLength);
        VXIO_ASSERT(inStream.good());

        VXIO_ASSERT_EQ(actualContents.length(), contents.length());
        VXIO_ASSERT_EQ(actualContents, contents);
    }

    {
        std::string actualContents;
        auto inStream = openForReadOrFail(randomPath);
        VXIO_ASSERT(inStream.good());

        while (true) {
            actualContents += inStream.readStringUntil('\n');
            if (inStream.eof()) {
                break;
            }
            actualContents += '\n';
        }

        VXIO_ASSERT(not inStream.err());

        VXIO_ASSERT_EQ(actualContents.length(), contents.length());
        VXIO_ASSERT_EQ(actualContents, contents);
    }

    std::filesystem::remove(std::filesystem::path{randomPath});
}

VXIO_TEST(stream, readRandomFile_FileInputStream)
{
    constexpr size_t fileLength = 8192;

    const std::string randomPath = suggestRandomOutputPath();
    const std::string contents = makeRandomString(fileLength, StringType::BINARY);
    dumpString(randomPath, contents);

    auto stream = openForReadOrFail(randomPath);

    std::string actualContents = stream.readString(fileLength);

    VXIO_ASSERT_EQ(actualContents.length(), contents.length());
    VXIO_ASSERT_EQ(actualContents, contents);

    std::filesystem::remove(std::filesystem::path{randomPath});
}

VXIO_TEST(stream, readRandomFile_StdInputStream)
{
    constexpr size_t fileLength = 8192;

    const std::string randomPath = suggestRandomOutputPath();
    const std::string contents = makeRandomString(fileLength, StringType::BINARY);
    dumpString(randomPath, contents);

    std::ifstream stdStream{randomPath};
    voxelio::StdInputStream voxelioStream{stdStream};
    VXIO_ASSERT(voxelioStream.good());

    std::string actualContents = voxelioStream.readString(fileLength);

    VXIO_ASSERT_EQ(actualContents.length(), contents.length());
    VXIO_ASSERT_EQ(actualContents, contents);

    std::filesystem::remove(std::filesystem::path{randomPath});
}

}  // namespace voxelio::test
