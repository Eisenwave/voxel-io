#ifndef VXIO_XYZRGB_HPP
#define VXIO_XYZRGB_HPP

#include "voxelio/voxelio.hpp"

namespace voxelio::xyzrgb {

class Writer : public AbstractListWriter {
private:
    bool initialized = false;

public:
    Writer(OutputStream &ostream) : AbstractListWriter{ostream} {}

    [[nodiscard]] ResultCode init() noexcept final;
    [[nodiscard]] ResultCode write(Voxel32 buffer[], size_t bufferLength) noexcept final;

private:
    [[nodiscard]] ResultCode writeVoxel(Voxel32 voxel) noexcept;
};

}  // namespace voxelio::xyzrgb

#endif
