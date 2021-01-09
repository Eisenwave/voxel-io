#include "voxelio/format/cubeworld.hpp"

#include "voxelio/color.hpp"
#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"

namespace voxelio::cub {

[[nodiscard]] ReadResult Reader::init() noexcept
{
    if (initialized) {
        return {0, ResultCode::WARNING_DOUBLE_INIT};
    }
    stream.readLittle<3, u32>(size.data());
    VXIO_NO_EOF();

    initialized = true;

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    writeHelper.reset(buffer, bufferLength);
    return doRead();
}

[[nodiscard]] ReadResult Reader::read(Voxel32 buffer[], size_t bufferLength) noexcept
{
    writeHelper.reset(buffer, bufferLength);
    return doRead();
}

[[nodiscard]] ReadResult Reader::doRead() noexcept
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::cub::Reader::init()");
        return init();
    }

    for (; currentPos.z() < size.z(); ++currentPos.z()) {
        for (; currentPos.y() < size.y(); ++currentPos.y()) {
            for (; currentPos.x() < size.x(); ++currentPos.x()) {
                if (writeHelper.isFull()) {
                    return ReadResult::ok(writeHelper.voxelsWritten());
                }
                u8 rgb[3];
                stream.read(rgb, 3);
                VXIO_NO_EOF();
                if ((rgb[0] | rgb[1] | rgb[2]) == 0) {
                    // pure black is not possible to encode because it represents the absence of a voxel
                    // when we read pure black, we don't output any voxels
                    continue;
                }
                Vec3i32 pos = currentPos.cast<i32>();
                argb32 color = Color32{rgb[0], rgb[1], rgb[2]};
                writeHelper.emplace(pos, color);
            }
            currentPos.x() = 0;
        }
        currentPos.y() = 0;
    }

    return ReadResult::end(writeHelper.voxelsWritten());
}

}  // namespace voxelio::cub
