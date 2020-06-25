#include "vl32.hpp"

#include "log.hpp"
#include "macro.hpp"

namespace voxelio::vl32 {

[[nodiscard]] ReadResult Reader::init() noexcept
{
    if (initialized) {
        return {0, ResultCode::WARNING_DOUBLE_INIT};
    }
    initialized = true;

    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::vl32::Reader::init()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

[[nodiscard]] ReadResult Reader::read(Voxel32 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_LOG(DEBUG, "calling voxelio::vl32::Reader::init()");
        return init();
    }

    writeHelper.reset(buffer, bufferLength);

    return doRead();
}

[[nodiscard]] ReadResult Reader::doRead() noexcept
{
    Voxel32 voxel;
    while (not writeHelper.isFull()) {
        VXIO_FORWARD_ERROR(readVoxel(voxel));
        if (stream.eof()) {
            return ReadResult::end(writeHelper.voxelsWritten());
        }
        writeHelper.write(voxel);
    }
    return ReadResult::ok(writeHelper.voxelsWritten());
}

[[nodiscard]] ReadResult Reader::readVoxel(Voxel32 &out)
{
    stream.readBig<3, i32>(out.pos.data());
    out.argb = stream.readBig<u32>();

    if (stream.err()) {
        return ReadResult::ioError(stream.position(), "IO error when reading voxel");
    }

    return ReadResult::ok();
}

ResultCode Writer::init() noexcept
{
    if (initialized) {
        return ResultCode::WARNING_DOUBLE_INIT;
    }
    initialized = true;

    return ResultCode::OK_INITIALIZED;
}

ResultCode Writer::write(Voxel32 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_FORWARD_ERROR(init());
    }
    VXIO_DEBUG_ASSERT(initialized);

    for (size_t i = 0; i < bufferLength; ++i) {
        VXIO_FORWARD_ERROR(writeVoxel(buffer[i]));
    }
    return ResultCode::WRITE_OK;
}

ResultCode Writer::writeVoxel(Voxel32 v) noexcept
{
    stream.writeBig<v.pos.size, i32>(v.pos.data());
    stream.writeBig<argb32>(v.argb);
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

}  // namespace voxelio::vl32
