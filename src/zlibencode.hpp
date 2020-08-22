#ifndef ZLIBENCODE_HPP
#define ZLIBENCODE_HPP

#include "assert.hpp"
#include "primitives.hpp"
#include "stream.hpp"

#include "3rd_party/miniz.hpp"

namespace voxelio::zlib {

constexpr usize BUFFER_SIZE = 256 * 1024;

/** A result code. */
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

/** A compression strategy. */
enum class Strategy : int {
    DEFAULT = MZ_DEFAULT_STRATEGY,
    FILTERED = MZ_FILTERED,
    HUFFMAN_ONLY = MZ_HUFFMAN_ONLY,
    RLE = MZ_RLE,
    FIXED = MZ_FIXED
};

enum class Flushing : int {
    NONE = MZ_NO_FLUSH,
    PARTIAL = MZ_PARTIAL_FLUSH,
    SYNC = MZ_SYNC_FLUSH,
    FULL = MZ_FULL_FLUSH,
    FINISH = MZ_FINISH,
    BLOCK = MZ_BLOCK
};

inline const char* errorOf(ResultCode code)
{
    return mz_error(static_cast<int>(code));
}

class Deflator {
private:
    mz_stream stream{};
    u8 out[BUFFER_SIZE];

    Deflator() {
    }

public:
    Deflator(unsigned level) : Deflator{}
    {
        VXIO_ASSERT_LT(level, 10);
        ResultCode result{mz_deflateInit(&stream, static_cast<int>(level))};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    Deflator(unsigned level, unsigned windowBits, unsigned memLevel, Strategy strategy = Strategy::DEFAULT)
     : Deflator{}
    {
        VXIO_ASSERT_LT(level, 10);
        VXIO_ASSERT_NE(memLevel, 0);
        VXIO_ASSERT_LT(memLevel, 10);
        ResultCode result{mz_deflateInit2(&stream,
                        static_cast<int>(level),
                        MZ_DEFLATED,
                        static_cast<int>(windowBits),
                        static_cast<int>(memLevel),
                        static_cast<int>(strategy))};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    ~Deflator()
    {
        ResultCode result{mz_deflateEnd(&stream)};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    ResultCode deflate(u8 in[], usize size, OutputStream &outStream, Flushing flush = Flushing::NONE)
    {
        stream.next_in = in;
        stream.avail_in = static_cast<unsigned int>(size);

        while (stream.avail_in != 0) {
            stream.avail_out = BUFFER_SIZE;
            stream.next_out = out;

            ResultCode result{mz_deflate(&stream, static_cast<int>(flush))};
            VXIO_ASSERT_NE(result, ResultCode::STREAM_ERROR);

            usize have = BUFFER_SIZE - stream.avail_out;
            outStream.write(stream.next_out, have);
            if (not outStream.good()) {
                return ResultCode::STREAM_ERROR;
            }
        }

        return ResultCode::OK;
    }

    ResultCode reset()
    {
        return ResultCode{mz_deflateReset(&stream)};
    }

    ResultCode flush(OutputStream &outStream)
    {
        stream.avail_in = 0;
        //stream.next_in = nullptr; not necessary
        stream.next_out = out;
        stream.avail_out = BUFFER_SIZE;

        ResultCode result{mz_deflate(&stream, static_cast<int>(Flushing::FINISH))};
        VXIO_ASSERT_NE(result, ResultCode::STREAM_ERROR);

        usize have = BUFFER_SIZE - stream.avail_out;
        outStream.write(stream.next_out, have);

        return outStream.good() ? ResultCode::OK : ResultCode::STREAM_ERROR;
    }

};

class Inflator {
private:
    mz_stream stream;

    void initStream()
    {

    }

public:
    Inflator()
    {
        initStream();
        mz_inflateInit(&stream);
    }

    Inflator(unsigned windowBits)
    {
        initStream();
        mz_inflateInit2(&stream, static_cast<int>(windowBits));
    }

    ~Inflator()
    {
        ResultCode result{mz_inflateEnd(&stream)};
        VXIO_ASSERT_EQ(result, ResultCode::OK);
    }

    // ENCODER MANAGEMENT ==============================================================================================

    mz_ulong deflateBound(mz_ulong sourceLength)
    {
        return mz_deflateBound(&stream, sourceLength);
    }

    ResultCode reset()
    {
        return ResultCode{mz_deflateReset(&stream)};
    }

    // COMPRESSION =====================================================================================================

    ResultCode compress(const u8* source, mz_ulong sourceLength, u8 outDest[], mz_ulong &outDestLen)
    {
        return ResultCode{mz_compress(outDest, &outDestLen, source, sourceLength)};
    }

    ResultCode compress(const u8* source, mz_ulong sourceLength, u8 outDest[], mz_ulong &outDestLen, unsigned level)
    {
        return ResultCode{mz_compress2(outDest, &outDestLen, source, sourceLength, static_cast<int>(level))};
    }

};

}  // namespace voxelio::zlib

#endif  // ZLIBENCODE_HPP
