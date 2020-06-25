#ifndef VXIO_VL32_HPP
#define VXIO_VL32_HPP

#include "../types.hpp"
#include "../util_public.hpp"
#include "../voxelio.hpp"

namespace voxelio {
namespace vl32 {

class Reader : public AbstractReader {
private:
    VoxelBufferWriteHelper writeHelper;
    bool initialized = false;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;
    [[nodiscard]] ReadResult read(Voxel32 buffer[], size_t bufferLength) noexcept;

private:
    [[nodiscard]] ReadResult doRead() noexcept;
    [[nodiscard]] ReadResult readVoxel(Voxel32 &out);
};

class Writer : public AbstractListWriter {
private:
    bool initialized = false;

public:
    Writer(OutputStream &ostream) : AbstractListWriter{ostream} {}

    [[nodiscard]] ResultCode init() noexcept override;
    [[nodiscard]] ResultCode write(Voxel32 buffer[], size_t bufferLength) noexcept override;

private:
    [[nodiscard]] ResultCode writeVoxel(Voxel32 voxel) noexcept;
};

}  // namespace vl32
}  // namespace voxelio

#endif  // VOXELIO_VL32_HPP
