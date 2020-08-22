#ifndef VXIO_DEFLATE_HPP
#define VXIO_DEFLATE_HPP
/*
 * deflate.hpp
 * -----------
 * Provides a safe object-oriented Zlib abstraction.
 * Deflate happens using Deflator.
 * Inflate happens using Inflator.
 *
 */

#include "assert.hpp"
#include "primitives.hpp"
#include "stream.hpp"

#include "3rd_party/miniz/miniz.h"

namespace voxelio::deflate {

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

    /**
     * @brief Returns the total number of bytes read.
     * @return the total number of bytes read
     */
    [[nodiscard]] u64 totalRead() const
    {
        return zStream.total_in;
    }

    /**
     * @brief Returns the total number of bytes written.
     * @return the total number of bytes written
     */
    [[nodiscard]] u64 totalWritten() const
    {
        return zStream.total_out;
    }

    /**
     * @brief Resets the deflator so that it can be used for encoding another data stream.
     * @return the result code
     */
    [[nodiscard]] ResultCode reset()
    {
        return ResultCode{mz_deflateReset(&zStream)};
    }

    /**
     * @brief deflates the next array of bytes
     * @param in the input array
     * @param size the size of the input
     * @param flush the flushing mode
     * @return the result code
     */
    [[nodiscard]] ResultCode deflate(u8 in[], usize size, Flushing flush = Flushing::NONE);

    /**
     * @brief Flushes the deflator.
     * @return the result code
     */
    [[nodiscard]] ResultCode flush();
};

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

    /**
     * @brief Returns the total number of bytes read.
     * @return the total number of bytes read
     */
    [[nodiscard]] u64 totalRead() const
    {
        return zStream.total_in;
    }

    /**
     * @brief Returns the total number of bytes written.
     * @return the total number of bytes written
     */
    [[nodiscard]] u64 totalWritten() const
    {
        return zStream.total_out;
    }

    /**
     * @brief Returns whether the end of the stream has been reached.
     * @return true if the end of the stream was reached
     */
    bool eof() const
    {
        return eof_;
    }

    /**
     * @brief Inflates the next bit of data and stores the result in the given byte buffer.
     * @param out the output byte buffer
     * @param size the size of the output buffer
     * @param outWritten the actual number of bytes which were written to the buffer
     * @return the result code
     */
    [[nodiscard]] ResultCode inflate(u8 out[], usize size, usize &outWritten);
};

}  // namespace voxelio::deflate

#endif  // ZLIBENCODE_HPP
