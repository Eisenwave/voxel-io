#include "voxelio/test/io.hpp"
#include "voxelio/test/random.hpp"

#include "voxelio/stream.hpp"

#include <filesystem>

namespace voxelio::test {

static std::uniform_int_distribution<unsigned> distributionOf(StringType type)
{
    switch (type) {
    case StringType::BYTE: return std::uniform_int_distribution<unsigned>{0, 0xff};
    case StringType::ASCII: return std::uniform_int_distribution<unsigned>{0, 127};
    case StringType::PRINTABLE: return std::uniform_int_distribution<unsigned>{' ', '~'};
    case StringType::LOWER_CHARS: return std::uniform_int_distribution<unsigned>{'a', 'z'};
    case StringType::UPPER_CHARS: return std::uniform_int_distribution<unsigned>{'A', 'Z'};
    case StringType::BINARY: return std::uniform_int_distribution<unsigned>{'0', '1'};
    case StringType::DECIMAL: return std::uniform_int_distribution<unsigned>{'0', '9'};
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

std::string makeRandomString(size_t length, StringType type, u32 seed, u32 charRepititions)
{
    default_rng rng{seed};
    std::uniform_int_distribution<unsigned> distr = distributionOf(type);

    std::string result;
    result.resize(length * charRepititions);

    for (size_t i = 0; i < length; ++i) {
        size_t base = i * charRepititions;
        for (size_t j = 0; j < charRepititions; ++j) {
            result[base + j] = static_cast<char>(distr(rng));
        }
    }
    return result;
}

static std::string initOutputDirectory()
{
    std::filesystem::path tmpBasePath = std::filesystem::temp_directory_path();
    std::filesystem::path tmpPath = tmpBasePath.concat("/voxelio-test/");

    VXIO_ASSERT_CONSEQUENCE(std::filesystem::exists(tmpPath), std::filesystem::is_directory(tmpPath));
    std::filesystem::create_directories(tmpPath);

    return tmpPath.string();
}

const std::string &getOutputPath()
{
    static const std::string tmpPath = initOutputDirectory();
    return tmpPath;
}

void dumpString(const std::string &path, const std::string &str)
{
    std::optional<FileOutputStream> stream = FileOutputStream::open(path, OpenMode::BINARY);
    VXIO_ASSERT(stream.has_value());

    stream->writeString(str);
}

}  // namespace voxelio::test
