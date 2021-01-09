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

#include "3rd_party/miniz.h"

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

enum Defaults : unsigned {
    DEFAULT_LEVEL = MZ_DEFAULT_LEVEL,
    DEFAULT_WINDOW_BITS = 15,
    DEFAULT_MEM_LEVEL = 9,
};

/// Stores compression settings for deflation.
struct DeflateSettings {
    /**
     * The compression level in [0, 9] sets the quality of compression vs speed:
     * <ul>
     *     <li>0 gives no compression at all,</li>
     *     <li>1 gives best speed,</li>
     *     <li>9 gives best compression</li>
     * </ul>
     */
    unsigned level = DEFAULT_LEVEL;
    /**
     * The windowBits parameter is the base two logarithm of the window size (the size of the history buffer).
     * It should be in (8, 16), the default is 15.
     */
    unsigned windowBits = DEFAULT_WINDOW_BITS;
    /**
     * The memLevel parameter specifies how much memory should be allocated for the internal compression state.
     * - 1 uses minimum memory but is slow and reduces compression ratio
     * - 9 uses maximum memory for optimal speed.
     * The default value is 8.
     */
    unsigned memLevel = DEFAULT_MEM_LEVEL;
    /**
     * The compression strategy.
     */
    Strategy strategy = Strategy::DEFAULT;

    constexpr bool isValid()
    {
        return level < 10 && windowBits < 16 && memLevel < 10;
    }
};

class Deflator {
private:
    mz_stream zStream{};
    OutputStream &oStream;
    u8 out[BUFFER_SIZE];

public:
    Deflator(const Deflator &) = delete;
    Deflator(Deflator &&) = delete;

    /**
     * @brief Initializes the internal stream state for compression.
     *
     * @param stream the output stream
     * @param settings the compression settings
     */
    Deflator(OutputStream &stream, DeflateSettings settings = {});

    /**
     * @brief Initializes the internal stream state for compression.
     * Only sets the compression level and leaves everything else at default.
     *
     * @param stream the output stream
     * @param level the compression level [0, 9], defaults to 6
     */
    Deflator(OutputStream &stream, unsigned level) : Deflator{stream, DeflateSettings{level}} {}

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
     * @brief Deflates the next array of bytes.
     * If the given size is zero, this method exits quickly and always returns OK.
     *
     * @param in the input array
     * @param size the size of the input
     * @param flush the flushing mode
     * @return the result code
     */
    [[nodiscard]] ResultCode deflate(const u8 in[], usize size, Flushing flush = Flushing::NONE);

    [[nodiscard]] ResultCode deflate(u8 in, Flushing flush = Flushing::NONE)
    {
        return deflate(&in, 1, flush);
    }

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
     * If the given size is zero, this method exits quickly and always returns OK.
     *
     * @param out the output byte buffer
     * @param size the size of the output buffer
     * @param outWritten the actual number of bytes which were written to the buffer
     * @return the result code
     */
    [[nodiscard]] ResultCode inflate(u8 out[], usize size, usize &outWritten);
};

}  // namespace voxelio::deflate

#endif  // ZLIBENCODE_HPP
