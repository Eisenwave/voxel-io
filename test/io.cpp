#include "voxelio/test/io.hpp"
#include "voxelio/test/random.hpp"

#include "voxelio/filetype.hpp"
#include "voxelio/format/vl32.hpp"
#include "voxelio/ioutil.hpp"
#include "voxelio/palette.hpp"
#include "voxelio/stream.hpp"
#include "voxelio/voxelarray.hpp"
#include "voxelio/voxelio.hpp"

#include <algorithm>
#include <filesystem>

namespace voxelio::test {

static constexpr usize IO_VOXEL_BUFFER_SIZE = 8192;

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

void makeRandomData(u8 *out, usize length, u32 seed) noexcept
{
    default_rng rng{seed};
    std::uniform_int_distribution<unsigned> distr{0, 0xff};
    for (usize i = 0; i < length; ++i) {
        out[i] = static_cast<u8>(distr(rng));
    }
}

void makeRandomIndexSequence(usize *out, usize length, u32 seed) noexcept
{
    std::iota(out, out + length, 0);
    std::shuffle(out, out + length, default_rng{seed});
}

void makeRandomChars(char *out, usize length, StringType type, u32 seed, u32 charRepititions) noexcept
{
    default_rng rng{seed};
    std::uniform_int_distribution<unsigned> distr = distributionOf(type);

    for (usize i = 0; i < length; ++i) {
        usize base = i * charRepititions;
        for (usize j = 0; j < charRepititions; ++j) {
            out[base + j] = static_cast<char>(distr(rng));
        }
    }
}

std::string makeRandomString(usize length, StringType type, u32 seed, u32 charRepititions)
{
    std::string result(length * charRepititions, '\0');
    makeRandomChars(result.data(), length, type, seed, charRepititions);
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

FileInputStream openForReadOrFail(const std::string &path, OpenMode mode)
{
    FileInputStream result{path, mode};
    VXIO_ASSERTM(result.good(), "Failed to open file for read \"" + path + '"');
    return result;
}

FileOutputStream openForWriteOrFail(const std::string &path, OpenMode mode)
{
    FileOutputStream result{path, mode};
    VXIO_ASSERTM(result.good(), "Failed to open file for read \"" + path + '"');
    return result;
}

FileInputStream openTestAsset(const std::string &path, OpenMode mode)
{
    std::string fullPath = ASSET_PATH + ('/' + path);
    return openForReadOrFail(fullPath, mode);
}

const std::string &getOutputPath()
{
    static const std::string tmpPath = initOutputDirectory();
    return tmpPath;
}

void dumpString(const std::string &path, const std::string &str)
{
    FileOutputStream stream = openForWriteOrFail(path, OpenMode::BINARY);
    stream.writeString(str);
}

void pipeOrFail(voxelio::InputStream &input, voxelio::OutputStream &output)
{
    constexpr usize bufferSize = 8192;
    uint8_t buffer[bufferSize];

    do {
        usize read = input.read(buffer, bufferSize);
        VXIO_ASSERT(not input.err());
        output.write(buffer, read);
        VXIO_ASSERT(not output.err());
    } while (not input.eof());
}

uintmax_t getFileSize(const std::string &file)
{
    return std::filesystem::file_size(file);
}

// ITERATIVE WRITING ===================================================================================================

template <bool PALETTE>
usize writeIteratively_impl(AbstractListWriter &writer, const VoxelArray &voxels)
{
    auto buffer = std::make_unique<Voxel32[]>(IO_VOXEL_BUFFER_SIZE);
    auto bufferedWriter = ListWriterWriteHelper32{writer, buffer.get(), IO_VOXEL_BUFFER_SIZE};
    auto &palette = writer.palette();

    usize count = 0;
    for (const Voxel32 voxel : voxels) {
        argb32 color = voxel.argb;
        if constexpr (PALETTE) {
            color = palette.indexOf(color);
        }
        VXIO_ASSERT(isGood(bufferedWriter.write(voxel.pos, color)));
        ++count;
    }

    VXIO_ASSERT(isGood(bufferedWriter.flush()));
    return count;
}

usize writeIteratively(AbstractListWriter &writer, const VoxelArray &voxels)
{
    return writeIteratively_impl<false>(writer, voxels);
}

usize writeIteratively(const std::string &path, FileType type, const VoxelArray &voxels)
{
    FileOutputStream stream = openForWriteOrFail(path);

    VXIO_ASSERT_EQ(type, FileType::VL32);
    vl32::Writer writer{stream};

    return writeIteratively(writer, voxels);
}

usize writeIteratively(const std::string &path, const VoxelArray &voxels)
{
    auto fileType = detectFileTypeUsingName(path);
    VXIO_ASSERT(fileType.has_value());
    return writeIteratively(path, *fileType, voxels);
}

usize writeIterativelyUsingPalette(AbstractListWriter &writer, const VoxelArray &voxels)
{
    return writeIteratively_impl<true>(writer, voxels);
}

// ITERATIVE READING ===================================================================================================

template <typename Consumer, std::enable_if_t<std::is_invocable_v<Consumer, Voxel32>, int> = 0>
static void readIteratively_impl(AbstractReader &reader, Consumer consumer)
{
    {
        auto buffer = std::make_unique<Voxel64[]>(IO_VOXEL_BUFFER_SIZE);
        ReadResult result;
        do {
            result = reader.read(buffer.get(), IO_VOXEL_BUFFER_SIZE);
            VXIO_ASSERT(isGood(result));

            for (usize i = 0; i < result.voxelsRead; ++i) {
                auto outPos = buffer[i].pos.cast<usize>();
                auto outRgb = static_cast<argb32>(buffer[i].argb);
                consumer(Voxel32{outPos.cast<int32_t>(), {outRgb}});
            }

        } while (not result.isEnd());
    }
}

void readIteratively(AbstractReader &reader, std::vector<Voxel32> &out)
{
    readIteratively_impl(reader, [&out](Voxel32 voxel) {
        out.push_back(std::move(voxel));
    });
}

void readIteratively(AbstractReader &reader, VoxelArray &out)
{
    readIteratively_impl(reader, [&out](Voxel32 voxel) {
        for (usize i = 0; i < 3; ++i) {
            VXIO_DEBUG_ASSERT_GE(voxel.pos[i], 0);
        }

        Vec3u32 upos = voxel.pos.cast<uint32_t>();
        out[{upos[0], upos[1], upos[2]}] = voxel.argb;
    });
}

void readIterativelyAndMoveToOrigin(AbstractReader &reader, VoxelArray &out)
{
    std::vector<Voxel32> cache;
    readIteratively(reader, cache);
    VXIO_ASSERT(not cache.empty());

    // TODO use only voxelio here
    auto min = Vec3i32::filledWith(std::numeric_limits<i32>::max());
    for (const auto &voxel : cache) {
        for (usize i = 0; i < 3; ++i) {
            min[i] = std::min(min[i], voxel.pos[i]);
        }
    }
    Vec3i32 min_vxio = min;
    for (const Voxel32 voxel : cache) {
        Vec3size upos = (voxel.pos - min_vxio).cast<usize>();
        out[upos] = voxel.argb;
    }
}

}  // namespace voxelio::test
