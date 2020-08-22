#ifndef ZLIBENCODE_HPP
#define ZLIBENCODE_HPP

#include "assert.hpp"
#include "primitives.hpp"
#include "stream.hpp"

#include "3rd_party/miniz.hpp"

namespace voxelio::zlib {

constexpr usize BUFFER_SIZE = 256 * 1024;

/// A result code.
enum class ResultCode : int {
    OK = MZ_OK,
    STREAM_END = MZ_STREAM_END,
    NEED_DICT = MZ_NEED_DICT,
    ERRNO = MZ_ERRNO,
    STREAM_ERROR = MZ_STREAM_ERROR,
    DATA_ERROR = MZ_DATA_ERROR,
    MEM_ERROR = MZ_MEM_ERROR,
    BUF_ERROR = MZ_BUF_ERROR,
    VERSION_ERROR = MZ_VERSION_ERROR,
    PARAM_ERROR = MZ_PARAM_ERROR
};

/// A compression strategy.
enum class Strategy : int {
    DEFAULT = MZ_DEFAULT_STRATEGY,
    FILTERED = MZ_FILTERED,
    HUFFMAN_ONLY = MZ_HUFFMAN_ONLY,
    RLE = MZ_RLE,
    FIXED = MZ_FIXED
};

/// A flushing method.
enum class Flushing : int {
    NONE = MZ_NO_FLUSH,
    PARTIAL = MZ_PARTIAL_FLUSH,
    SYNC = MZ_SYNC_FLUSH,
    FULL = MZ_FULL_FLUSH,
    FINISH = MZ_FINISH,
    BLOCK = MZ_BLOCK
};

inline const char *errorOf(ResultCode code)
{
    return mz_error(static_cast<int>(code));
}

// DEFLATOR ============================================================================================================

class Deflator {
private:
    mz_stream zStream{};
    OutputStream &oStream;
    u8 out[BUFFER_SIZE];

public:
    Deflator(const Deflator &) = delete;
    Deflator(Deflator &&) = delete;

    Deflator(OutputStream &stream, unsigned level);
    Deflator(OutputStream &stream,
             unsigned level,
             unsigned windowBits,
             unsigned memLevel,
             Strategy strategy = Strategy::DEFAULT);

    ~Deflator()
    {
        ResultCode result{mz_deflateEnd(&zStream)};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    [[nodiscard]] u64 totalRead() const
    {
        return zStream.total_in;
    }

    [[nodiscard]] u64 totalWritten() const
    {
        return zStream.total_out;
    }

    [[nodiscard]] ResultCode deflate(u8 in[], usize size, Flushing flush = Flushing::NONE);

    [[nodiscard]] ResultCode reset()
    {
        return ResultCode{mz_deflateReset(&zStream)};
    }

    [[nodiscard]] ResultCode flush();
};

Deflator::Deflator(OutputStream &stream, unsigned level) : oStream{stream}
{
    VXIO_ASSERT_LT(level, 10);
    ResultCode result{mz_deflateInit(&zStream, static_cast<int>(level))};
    VXIO_ASSERT_EQ(result, ResultCode::OK);
}

Deflator::Deflator(OutputStream &stream, unsigned level, unsigned windowBits, unsigned memLevel, Strategy strategy)
    : oStream{stream}
{
    VXIO_ASSERT_LT(level, 10);
    VXIO_ASSERT_NE(memLevel, 0);
    VXIO_ASSERT_LT(memLevel, 10);
    ResultCode result{mz_deflateInit2(&zStream,
                                      static_cast<int>(level),
                                      MZ_DEFLATED,
                                      static_cast<int>(windowBits),
                                      static_cast<int>(memLevel),
                                      static_cast<int>(strategy))};
    VXIO_ASSERT_EQ(result, ResultCode::OK);
}

[[nodiscard]] ResultCode Deflator::deflate(u8 in[], usize size, Flushing flushing)
{
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

class Inflator {
private:
    mz_stream zStream{};
    InputStream &iStream;
    u8 in[BUFFER_SIZE];
    bool eof_ = false;

public:
    Inflator(InputStream &stream) : iStream{stream}
    {
        ResultCode result{mz_inflateInit(&zStream)};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    Inflator(InputStream &stream, unsigned windowBits) : iStream{stream}
    {
        ResultCode result{mz_inflateInit2(&zStream, static_cast<int>(windowBits))};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    ~Inflator()
    {
        ResultCode result{mz_inflateEnd(&zStream)};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    [[nodiscard]] u64 totalRead() const
    {
        return zStream.total_in;
    }

    [[nodiscard]] u64 totalWritten() const
    {
        return zStream.total_out;
    }

    bool eof() const
    {
        return eof_;
    }

    [[nodiscard]] ResultCode inflate(u8 out[], usize size, usize &written);
};

[[nodiscard]] ResultCode Inflator::inflate(u8 out[], usize size, usize& written)
{
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

}  // namespace voxelio::zlib

#endif  // ZLIBENCODE_HPP
