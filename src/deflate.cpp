#include "voxelio/deflate.hpp"

namespace voxelio::deflate {

// DEFLATOR ============================================================================================================

Deflator::Deflator(OutputStream &stream, DeflateSettings settings) : oStream{stream}
{
    VXIO_ASSERT_LT(settings.level, 10u);
    VXIO_ASSERT_NE(settings.memLevel, 0u);
    VXIO_ASSERT_LT(settings.memLevel, 10u);
    ResultCode result{mz_deflateInit2(&zStream,
                                      static_cast<int>(settings.level),
                                      MZ_DEFLATED,
                                      static_cast<int>(settings.windowBits),
                                      static_cast<int>(settings.memLevel),
                                      static_cast<int>(settings.strategy))};
    VXIO_ASSERT_EQ(result, ResultCode::OK);
}

[[nodiscard]] ResultCode Deflator::deflate(const u8 in[], usize size, Flushing flushing)
{
    VXIO_ASSERT_NOTNULL(in);

    zStream.next_in = in;
    zStream.avail_in = static_cast<unsigned int>(size);

    // while input not exhausted
    while (zStream.avail_in != 0) {
        zStream.avail_out = BUFFER_SIZE;
        zStream.next_out = this->out;

        // fill as much of the output buffer as possible
        ResultCode result{mz_deflate(&zStream, static_cast<int>(flushing))};
        if (result != ResultCode::OK) {
            return result;
        }

        // write all the writen output (if any) through to our voxel-io stream
        usize written = BUFFER_SIZE - zStream.avail_out;
        if (written != 0) {
            oStream.write(this->out, written);
            if (not oStream.good()) {
                return ResultCode::STREAM_ERROR;
            }
        }
    }

    return ResultCode::OK;
}

[[nodiscard]] ResultCode Deflator::flush()
{
    zStream.avail_in = 0;
    // stream.next_in = nullptr; not necessary
    zStream.next_out = out;
    zStream.avail_out = BUFFER_SIZE;

    ResultCode result{mz_deflate(&zStream, static_cast<int>(Flushing::FINISH))};
    VXIO_ASSERT_NE(result, ResultCode::STREAM_ERROR);

    usize have = BUFFER_SIZE - zStream.avail_out;
    oStream.write(this->out, have);

    return oStream.good() ? ResultCode::OK : ResultCode::STREAM_ERROR;
}

// INFLATOR ============================================================================================================

[[nodiscard]] ResultCode Inflator::inflate(u8 out[], usize size, usize &written)
{
    VXIO_ASSERT_NOTNULL(out);

    zStream.next_out = out;
    zStream.avail_out = static_cast<unsigned int>(size);

    // loop until output buffer is completely filled
    while (zStream.avail_out != 0) {
        if (zStream.avail_in == 0) {
            // If the input buffer is larger than the output, we might not have exhausted our input during the
            // previous call.
            // Now we first need to write our buffered data before we fetch any more from the input stream.
            // If we have exhausted our input buffer, we can fetch more data from the input stream.
            usize read = iStream.read(in, BUFFER_SIZE);
            if (iStream.err()) {
                return ResultCode::STREAM_ERROR;
            }
            zStream.next_in = in;
            zStream.avail_in = static_cast<unsigned int>(read);
        }

        ResultCode result{mz_inflate(&zStream, static_cast<int>(Flushing::NONE))};
        if (result == ResultCode::STREAM_END) {
            written = size - zStream.avail_out;
            this->eof_ = true;
            return ResultCode::OK;
        }
        else if (result != ResultCode::OK) {
            return result;
        }
    }

    written = size - zStream.avail_out;
    return ResultCode::OK;
}

}  // namespace voxelio::deflate
