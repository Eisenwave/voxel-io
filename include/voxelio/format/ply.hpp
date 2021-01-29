#ifndef VXIO_PLY_HPP
#define VXIO_PLY_HPP

#include "voxelio/voxelio.hpp"
#include "voxelio/stream.hpp"

namespace voxelio::ply {

class Writer final : public AbstractListWriter {
private:
    bool initialized = false;
    u32 voxelCount = 0;
    u64 vertexCountOffset;

public:
    Writer(OutputStream &ostream) : AbstractListWriter{ostream} {}
    ~Writer() final;

    [[nodiscard]] ResultCode init() noexcept final;
    [[nodiscard]] ResultCode write(Voxel32 buffer[], size_t bufferLength) noexcept final;

private:
    [[nodiscard]] ResultCode writeVoxel(Voxel32 voxel) noexcept;
};

}

#endif
