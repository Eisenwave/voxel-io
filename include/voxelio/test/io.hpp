#ifndef VXIO_TEST_IO_HPP
#define VXIO_TEST_IO_HPP

#include "voxelio/test/random.hpp"

#include "voxelio/filetype.hpp"
#include "voxelio/fstream.hpp"
#include "voxelio/primitives.hpp"
#include "voxelio/voxelarray.hpp"
#include "voxelio/voxelio.hpp"

namespace voxelio::test {

#ifndef VXIO_TEST_ASSET_PATH
#error "VXIO_TEST_ASSET_PATH must be defined"
#endif

// two layers of evaluation necessary because for #arg or ##arg no macro substitution takes place
#define VXIO_IN_QUOTES_IMPL(arg) #arg
#define VXIO_IN_QUOTES(arg) VXIO_IN_QUOTES_IMPL(arg)

constexpr const char *ASSET_PATH = VXIO_IN_QUOTES(VXIO_TEST_ASSET_PATH);

[[nodiscard]] std::string makeRandomString(usize length,
                                           StringType type = StringType::PRINTABLE,
                                           u32 seed = 12345,
                                           u32 charRepititions = 1);

void dumpString(const std::string &path, const std::string &str);

[[nodiscard]] FileInputStream openForReadOrFail(const std::string &path, OpenMode = OpenMode::READ);

[[nodiscard]] FileOutputStream openForWriteOrFail(const std::string &path, OpenMode = OpenMode::WRITE);

[[nodiscard]] FileInputStream openTestAsset(const std::string &path, OpenMode = OpenMode::READ);

[[nodiscard]] const std::string &getOutputPath();

[[nodiscard]] inline std::string suggestRandomOutputPath()
{
    return getOutputPath() + makeRandomString(16, StringType::LOWER_CHARS, hardwareSeed());
}

void pipeOrFail(voxelio::InputStream &input, voxelio::OutputStream &output);

uintmax_t getFileSize(const std::string &file);

usize writeIteratively(AbstractListWriter &writer, const VoxelArray &voxels);

usize writeIteratively(const std::string &path, FileType type, const VoxelArray &voxels);

usize writeIteratively(const std::string &path, const VoxelArray &voxels);

usize writeIterativelyUsingPalette(AbstractListWriter &writer, const VoxelArray &voxels);

// ITERATIVE READING ===================================================================================================

void readIteratively(AbstractReader &reader, std::vector<Voxel32> &out);

void readIteratively(AbstractReader &reader, VoxelArray &out);

void readIterativelyAndMoveToOrigin(AbstractReader &reader, VoxelArray &out);

}  // namespace voxelio::test

#endif  // IO_HPP
