#include "voxelio/format/vl32.hpp"

#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"

namespace voxelio::vl32 {

[[nodiscard]] ReadResult Reader::init() noexcept
{
    return ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::read(Voxel64 buffer[], size_t bufferLength) noexcept
{
    return read_impl(buffer, bufferLength);
}

[[nodiscard]] ReadResult Reader::read(Voxel32 buffer[], size_t bufferLength) noexcept
{
    return read_impl(buffer, bufferLength);
}

template <typename Voxel>
ReadResult Reader::read_impl(Voxel buffer[], size_t bufferLength) noexcept
{
    Voxel32 voxel;
    size_t voxelsWritten = 0;

    for (; voxelsWritten < bufferLength; ++voxelsWritten) {
        VXIO_FORWARD_ERROR(readVoxel(voxel));
        if (stream.eof()) {
            return ReadResult::end(voxelsWritten);
        }
        buffer[voxelsWritten] = voxelCast<Voxel>(voxel);
    }

    return ReadResult::ok(voxelsWritten);
}

[[nodiscard]] ReadResult Reader::reset() noexcept
{
    stream.clearErrors();
    stream.seekAbsolute(0);
    return stream.err() ? ReadResult::ioError(0, "Failed to seek 0") : ReadResult::ok();
}

[[nodiscard]] ReadResult Reader::readVoxel(Voxel32 &out)
{
    i32 data[4];
    stream.readBig<4, i32>(data);
    out.pos = {data[0], data[1], data[2]};
    out.argb = static_cast<u32>(data[3]);

    return stream.err() ? ReadResult::ioError(stream.position(), "IO error when reading voxel") : ReadResult::ok();
}

ResultCode Writer::init() noexcept
{
    if (isInitialized()) {
        return ResultCode::WARNING_DOUBLE_INIT;
    }
    if (isFinalized()) {
        return ResultCode::USER_ERROR_INIT_AFTER_FINALIZE;
    }
    state = IoState::INITIALIZED;

    return ResultCode::OK;
}

ResultCode Writer::write(const Voxel32 buffer[], size_t bufferLength) noexcept
{
    if (not isInitialized()) {
        VXIO_FORWARD_ERROR(init());
    }
    VXIO_DEBUG_ASSERT(isInitialized());

    for (size_t i = 0; i < bufferLength; ++i) {
        VXIO_FORWARD_ERROR(writeVoxel(buffer[i]));
    }
    return ResultCode::OK;
}

ResultCode Writer::writeVoxel(Voxel32 v) noexcept
{
    stream.writeBig<v.pos.size, i32>(v.pos.data());
    stream.writeBig<argb32>(v.argb);
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

}  // namespace voxelio::vl32
