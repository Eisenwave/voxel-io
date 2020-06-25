#ifndef VXIO_CUBEWORLD_HPP
#define VXIO_CUBEWORLD_HPP

#include "../stream.hpp"
#include "../util_public.hpp"
#include "../voxelio.hpp"

namespace voxelio::cub {

class Reader : public AbstractReader {
private:
    VoxelBufferWriteHelper writeHelper;
    Vec3u32 size;
    Vec3u32 currentPos = Vec3u32::zero();
    bool initialized = false;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;
    [[nodiscard]] ReadResult read(Voxel32 buffer[], size_t bufferLength) noexcept;

private:
    [[nodiscard]] ReadResult doRead() noexcept;
};

}  // namespace voxelio::cub

#endif  // CUBEWORLD_HPP
