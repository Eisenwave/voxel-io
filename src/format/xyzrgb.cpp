#include "voxelio/format/xyzrgb.hpp"

#include "voxelio/color.hpp"
#include "voxelio/macro.hpp"

namespace voxelio::xyzrgb {

ResultCode Writer::init() noexcept
{
    if (initialized) {
        return ResultCode::WARNING_DOUBLE_INIT;
    }
    initialized = true;

    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

ResultCode Writer::write(const Voxel32 buffer[], usize bufferLength) noexcept
{
    if (not initialized) {
        VXIO_FORWARD_ERROR(init());
    }
    VXIO_DEBUG_ASSERT(initialized);

    for (usize i = 0; i < bufferLength; ++i) {
        VXIO_FORWARD_ERROR(writeVoxel(buffer[i]));
    }
    return ResultCode::OK;
}

ResultCode Writer::writeVoxel(Voxel32 v) noexcept
{
    for (usize i = 0; i < 3; ++i) {
        stream.writeString(stringifyDec(v.pos[i]));
        stream.write(' ');
    }

    Vec3u8 color = Color32{v.argb}.vec();
    for (usize i = 0; i < 3; ++i) {
        stream.writeString(stringifyDec(color[i]));
        stream.write(i == 2 ? '\n' : ' ');
    }

    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

}  // namespace voxelio::xyzrgb
