#ifndef VXIO_STREAM_HPP
#define VXIO_STREAM_HPP
/*
 * stream.hpp
 * -----------
 * Provides Java-style input/output streams.
 *
 * The <iostream> library has many issues which made the creation of this header necessary:
 * 1. <iostream> is an extremely heavyweight include and bloats compile times
 * 2. there is no portable way of obtaining and manipulating file positions (tellg() only does this on Linux and only
 *    for files opened in binary mode)
 * 3. implementing new stream types such as a NullOutputStream is extremely cumbersome due to implementation details
 *    like stream buffers, etc. which all have to be kept in mind
 * 4. having a dedicated stream allows for implementing endian encoding/decoding directly in the stream which is much
 *    more convenient than having to use wrapper functions
 */

#include "streamfwd.hpp"

#include "assert.hpp"
#include "endian.hpp"
#include "primitives.hpp"
#include "util.hpp"

#include <cstddef>

namespace voxelio {

using build::Endian;

template <typename T>
constexpr bool isIoType = std::is_arithmetic_v<T> || std::is_enum_v<T>;

/**
 * @brief Java-style implementation of an input stream.
 * In addition to a collection of abstract functions which provide input functionality, this class also contains flags
 * for fast error checking similar to C++ streams.
 * Numerous utility member functions are included in this stream which allow decoding data in various formats.
 */
class InputStream {
protected:
    /// Stores basic error flags for fast error checking.
    struct Flags {
        /// true if the end of file (EOF) was reached
        bool eof : 1;
        /// true if a read operation failed
        bool err : 1;
    };

    /// Small buffer used for decoding (groups of) integers in the writeU16, writeI32, etc. convenience functions
    u8 readBuffer[sizeof(umax) * 4 - sizeof(Flags)]{};
    Flags flags;

public:
    // VIRTUAL ---------------------------------------------------------------------------------------------------------

    InputStream() = default;
    InputStream(const InputStream &) = default;
    InputStream(InputStream &&) = default;
    virtual ~InputStream();

    /**
     * @brief Reads a single byte from the stream.
     * May set eof and err.
     * @return the byte or an undefined result if extraction failed
     */
    virtual u8 read() = 0;

    /**
     * @brief Reads at most maxSize characters into the buffer and returns the actual number of characters read.
     * The return value may be less than maxSize of eof is reached or an error occurrs.
     * If an error occurs, zero is returned.
     * Check err() in such a case.
     *
     * May set eof and err.
     * @param buffer the buffer
     * @param maxSize the maximum number of characters to read
     * @return the number of bytes actually read
     */
    virtual usize read(u8 buffer[], usize maxSize) = 0;

    /**
     * @brief Moves the read position to a given absolute position.
     * May set eof and err.
     * @param position the position to move to
     */
    virtual void seekAbsolute(u64 position) = 0;

    /**
     * @brief Moves the read position relatively by a given offset.
     * May set eof and err.
     * @param offset the offset from the current read position
     */
    virtual void seekRelative(i64 offset) = 0;

    /**
     * @brief Returns the current read position.
     * May set err.
     * @return the current read position
     */
    virtual u64 position() = 0;

    /**
     * @brief Resets the eof and err flag.
     * This is necessary when performing operations after one of these flags has been set.
     * For example, we need to clear error flags before seeking a position in the file after EOF has been reached.
     */
    virtual void clearErrors() = 0;

    /**
     * @brief Closes the stream.
     * @return whether the stream could be closed
     */
    virtual bool close() = 0;

    // DEFAULTS --------------------------------------------------------------------------------------------------------

    /**
     * @brief Reads bytes with a given maximum length until a delimiter.
     * The delimiter can be any byte.
     * May set err and eof.
     *
     * If the delimiter is not found in the read data, maxSize will be returned.
     * Otherwise the number of bytes before the delimiter is returned.
     * If the delimiter is the first read byte, then 0 is returned.
     *
     * Examples:
     * - if the maxSize is 1024 and the first byte is the delimiter, 0 is returned
     * - if the maxSize is 1024 and 10 bytes are read before the delimiter, 10 is returned
     * - if the maxSize is 1024 and the final byte is a delimiter, 1023 is returned
     * - if the maxSize is 1024 and no delimiter is found, 1024 is returned
     *
     * @param out the ouput string
     * @param length the maximum length
     * @param delimiter the delimiter
     * @return the number of bytes read
     */
    virtual usize read(u8 buffer[], usize size, u8 delimiter)
    {
        usize readCount = read(buffer, size);
        if (err()) {
            return 0;
        }

        // At this point we either reached the EOF or read all characters.
        // It doesn't really matter, we need to find the delimiter in the stream either way.
        for (usize i = 0; i < readCount; ++i) {
            if (buffer[i] == delimiter) {
                auto seekDistance = -static_cast<i64>(readCount - i - 1);
                if (seekDistance != 0 && eof()) {
                    clearErrors();
                }
                seekRelative(seekDistance);
                return i;
            }
        }
        return readCount;
    }

    /**
     * @brief Reads characters into a string until a delimiter or EOF is reached.
     * The delimiter is not included in the string.
     * May set eof and err.
     * @param out the string
     * @param delimiter the delimiter
     */
    virtual void readStringToUntil(std::string &out, char delimiter)
    {
        constexpr usize chunkSize = 128;
        const u8 delimiterByte = static_cast<u8>(delimiter);
        usize totalSize = 0;

        do {
            out.resize(totalSize + chunkSize);
            u8 *stringData = reinterpret_cast<u8 *>(out.data());
            usize readSize = read(stringData + totalSize, chunkSize, delimiterByte);
            totalSize += readSize;
            if (readSize != chunkSize) {
                out.resize(totalSize);
                return;
            }
        } while (good());
    }

    // FLAG HANDLING ---------------------------------------------------------------------------------------------------

    /**
     * @brief Returns true if the EOF (end of file) has been reached.
     * This flag will only be set when a read PAST the end of a file occurs.
     * When the final byte of a file is read correctly, this flag will not be set.
     * @return true if the EOF has been reached
     */
    bool eof() const noexcept
    {
        return flags.eof;
    }

    /**
     * @brief Returns true if a read operation failed.
     * @return true if a read operation failed
     */
    bool err() const noexcept
    {
        return flags.err;
    }

    /**
     * @brief Returns true if neither eof nor err are set.
     * @return true if neither eof nor err are set
     */
    bool good() const noexcept
    {
        return !eof() && !err();
    }

    // UTILITY ---------------------------------------------------------------------------------------------------------

    /**
     * @brief Reads a number of characters into a string.
     * May set eof and err.
     * @param out the string
     * @param length the number of characters to read
     */
    void readStringTo(std::string &out, usize length)
    {
        out.resize(length);
        read(reinterpret_cast<u8 *>(out.data()), length);
    }

    /**
     * @brief Reads a LF or CRLF terminated line.
     * Both cases are handled even when the stream is not opened in binary mode.
     * May set eof and err.
     *
     * This is to ensure that CRLF-terminated files coming from Windows systems can be interpreted on Unix systems
     * as well using this function.
     * There are also some OS-independent formats such as HTTP headers which use CRLF.
     *
     * @param out the string to write
     */
    void readLineTo(std::string &out)
    {
        readStringToUntil(out, '\n');
        if (not out.empty() && out.back() == '\r') {
            out.resize(out.size() - 1);
        }
    }

    /**
     * @brief Reads a LF or CRLF terminated line. Both cases are handled.
     * May set eof and err.
     * @return the read line
     */
    std::string readLine()
    {
        std::string result;
        readLineTo(result);
        return result;
    }

    /**
     * @brief Reads a LF or CRLF terminated line. Both cases are handled.
     * May set eof and err.
     * @return the read line
     */
    std::string readStringUntil(char delimiter = 0)
    {
        std::string result;
        readStringToUntil(result, delimiter);
        return result;
    }

    /**
     * @brief Reads a LF or CRLF terminated line. Both cases are handled.
     * May set eof and err.
     * @return the read line
     */
    std::string readString(usize length)
    {
        std::string result;
        readStringTo(result, length);
        return result;
    }

    /**
     * @brief Reads an unsigned 8-bit integer.
     * May set eof and err.
     * @return the read data
     */
    u8 readU8()
    {
        return readData<u8, Endian::NATIVE>();
    }

    /**
     * @brief Reads a signed 8-bit integer.
     * May set eof and err.
     * @return the read data
     */
    i8 readI8()
    {
        return readData<i8, Endian::NATIVE>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    T readBig()
    {
        return readData<T, Endian::BIG>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    T readLittle()
    {
        return readData<T, Endian::LITTLE>();
    }

    /**
     * @brief Reads an unsigned, native-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    T readNative()
    {
        return readData<T, Endian::NATIVE>();
    }

    /**
     * @brief Reads multiple unsigned, little-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void readLittle(T t[COUNT])
    {
        return readData<T, Endian::LITTLE, COUNT>(t);
    }

    /**
     * @brief Reads multiple unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void readBig(T t[COUNT])
    {
        return readData<T, Endian::BIG, COUNT>(t);
    }

    /**
     * @brief Reads multiple unsigned, native-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void readNative(T t[COUNT])
    {
        return readData<T, Endian::NATIVE, COUNT>(t);
    }

private:
    template <typename T, Endian ENDIAN>
    [[nodiscard]] T readData()
    {
        if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(readData<std::underlying_type_t<T>, ENDIAN>());
        }
        else {
            static_assert(sizeof(T) <= sizeof(u64));

            if constexpr (sizeof(T) == 1) {
                return static_cast<T>(read());
            }
            else {
                read(readBuffer, sizeof(T));
                return decode<ENDIAN, T>(readBuffer);
            }
        }
    }

    template <typename T, Endian ENDIAN, usize COUNT>
    void readDataToBuffer(T t[COUNT], u8 buffer[sizeof(T) * COUNT])
    {
        read(buffer, sizeof(T) * COUNT);

        for (usize i = 0; i < COUNT; ++i) {
            u8 *insertionPos = buffer + sizeof(T) * i;
            t[i] = decode<ENDIAN, T>(insertionPos);
        }
    }

    template <typename T, Endian ENDIAN, usize COUNT>
    void readData(T t[COUNT])
    {
        if constexpr (sizeof(T) == 1) {
            read(reinterpret_cast<u8 *>(t), COUNT);
        }
        else {
            constexpr usize bufferSize = sizeof(T) * COUNT;
            if constexpr (bufferSize <= sizeof(this->readBuffer)) {
                readDataToBuffer<T, ENDIAN, COUNT>(t, this->readBuffer);
            }
            else {
                u8 localBuffer[bufferSize];
                readDataToBuffer<T, ENDIAN, COUNT>(t, localBuffer);
            }
        }
    }
};

/**
 * @brief Java-style implementation of an output stream.
 * In addition to a collection of abstract functions which provide output functionality, this class also contains flags
 * for fast error checking similar to C++ streams.
 * Numerous utility member functions are included in this stream which allow encoding data in various formats.
 */
class OutputStream {
protected:
    /// Stores basic flags for fast error checking.
    struct Flags {
        /// true if a write-operation failed
        bool err : 1;
    };

    /// Small write buffer for writing (multiple) integers, used in the writeU16(), writeI32(), ... functions.
    u8 writeBuffer[sizeof(umax) * 4 - sizeof(Flags)]{};
    Flags flags;

public:
    // ABSTRACT --------------------------------------------------------------------------------------------------------

    OutputStream() = default;
    OutputStream(const OutputStream &) = default;
    OutputStream(OutputStream &&) = default;
    virtual ~OutputStream();

    /**
     * @brief Writes a single byte to the stream.
     * May set err.
     * @param byte the byte
     */
    virtual void write(u8 byte) = 0;

    /**
     * @brief Writes a buffer of bytes to the stream.
     * May set err.
     * @param buffer the bytes to write
     * @param size the number of bytes to write
     */
    virtual void write(const u8 buffer[], usize size) = 0;

    /**
     * @brief Moves the write position to a given absolute position.
     * May set err.
     * @param position the position to move to
     */
    virtual void seekAbsolute(u64 position) = 0;

    /**
     * @brief Moves the write position relatively by a given offset.
     * May set err.
     * @param offset the offset from the current write position
     */
    virtual void seekRelative(i64 offset) = 0;

    /**
     * @brief Returns the current write position.
     * May set err.
     * @return the current write position
     */
    virtual u64 position() = 0;

    /**
     * @brief Flushes the stream.
     * This will cause any remaining bytes to be written through to the backing output device.
     * May set err.
     */
    virtual void flush() = 0;

    /**
     * @brief Closes the stream.
     * @return whether the stream could be closed
     */
    virtual bool close() = 0;

    // DEFAULTS --------------------------------------------------------------------------------------------------------

    /**
     * @brief Writes a C-string to the stream.
     * May set err.
     * @param string the string
     */
    virtual void write(const char *string)
    {
        write(reinterpret_cast<const u8 *>(string), std::strlen(string));
    }

    // FLAG HANDLING ---------------------------------------------------------------------------------------------------

    /**
     * @brief Returns true if a write operation failed.
     * @return true if a write operation failed
     */
    bool err() const noexcept
    {
        return flags.err;
    }

    /**
     * @brief Returns true if no error occured.
     * @return true if no error occured.
     */
    bool good() const noexcept
    {
        return not err();
    }

    // UTILITY ---------------------------------------------------------------------------------------------------------

    /**
     * @brief Writes a string to the stream.
     * May set err.
     * @param str the string to write
     */
    void writeString(const std::string &str)
    {
        write(reinterpret_cast<const u8 *>(str.data()), str.size());
    }

    /**
     * @brief Writes a string followed by a line-ending to the stream.
     * On Windows, CRLF will be used if the file was opened in binary mode.
     * May set err.
     * @param str the line to write
     */
    void writeLine(const std::string &str)
    {
        writeString(str + '\n');
    }

    /**
     * @brief Writes a single 8-bit unsigned integer to the stream.
     * This is equivalent to write(u8) and just exists for consistency.
     * May set err.
     * @param data the data to write
     */
    void writeU8(u8 data)
    {
        write(data);
    }

    /**
     * @brief Writes a single 8-bit signed integer to the stream.
     * May set err.
     * @param data the data to write
     */
    void writeI8(i8 data)
    {
        write(static_cast<u8>(data));
    }

    /**
     * @brief Writes an unsigned, little-endian IO type.
     * May set err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeLittle(T t)
    {
        writeData<T, Endian::LITTLE>(t);
    }

    /**
     * @brief Writes an unsigned, big-endian IO type.
     * May set err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeBig(T t)
    {
        writeData<T, Endian::BIG>(t);
    }

    /**
     * @brief Writes an unsigned, native-endian IO type.
     * May set err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeNative(T t)
    {
        writeData<T, Endian::NATIVE>(t);
    }

    /**
     * @brief Writes multiple unsigned, big-endian IO types.
     * May set err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeLittle(const T t[COUNT])
    {
        writeData<T, Endian::LITTLE, COUNT>(t);
    }

    /**
     * @brief Writes multiple unsigned, big-endian IO types.
     * May set err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeBig(const T t[COUNT])
    {
        writeData<T, Endian::BIG, COUNT>(t);
    }

    /**
     * @brief Writes multiple unsigned, native-endian IO types.
     * May set err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T, std::enable_if_t<isIoType<T>, int> = 0>
    void writeNative(const T t[COUNT])
    {
        writeData<T, Endian::NATIVE, COUNT>(t);
    }

private:
    template <typename T, Endian ENDIAN>
    void writeData(T data)
    {
        if constexpr (std::is_enum_v<T>) {
            writeData<std::underlying_type_t<T>, ENDIAN>(static_cast<std::underlying_type_t<T>>(data));
        }
        else {
            static_assert(sizeof(T) <= sizeof(u64));

            if constexpr (sizeof(T) == 1) {
                write(static_cast<u8>(data));
            }
            else {
                encode<ENDIAN, T>(data, writeBuffer);
                write(writeBuffer, sizeof(T));
            }
        }
    }

    template <typename T, Endian ENDIAN, usize COUNT>
    void writeDataToBuffer(const T t[COUNT], u8 buffer[sizeof(T) * COUNT])
    {
        for (usize i = 0; i < COUNT; ++i) {
            u8 *insertionPos = buffer + sizeof(T) * i;
            encode<ENDIAN, T>(t[i], insertionPos);
        }

        write(buffer, sizeof(T) * COUNT);
    }

    template <typename T, Endian ENDIAN, usize COUNT>
    void writeData(const T data[COUNT])
    {
        if constexpr (sizeof(T) == 1) {
            write(reinterpret_cast<const u8 *>(data), COUNT);
        }
        else {
            constexpr usize bufferSize = sizeof(T) * COUNT;
            if constexpr (bufferSize <= sizeof(this->writeBuffer)) {
                writeDataToBuffer<T, ENDIAN, COUNT>(data, this->writeBuffer);
            }
            else {
                u8 localBuffer[bufferSize];
                writeDataToBuffer<T, ENDIAN, COUNT>(data, localBuffer);
            }
        }
    }
};

// NULL STREAMS ========================================================================================================

/**
 * @brief A special input stream that simulates an infinite file filled with zero-bytes.
 * It only keeps track of the position so that relative and absolute seeking is consistent.
 */
class NullInputStream final : public InputStream {
private:
    u64 pos = 0;

public:
    NullInputStream() noexcept
    {
        this->flags.eof = false;
        this->flags.err = false;
    }

    NullInputStream(NullInputStream &&) noexcept = default;
    ~NullInputStream() noexcept final = default;

    u8 read() final
    {
        pos++;
        return 0;
    }

    usize read(u8 buffer[], usize size) noexcept final
    {
        std::fill_n(buffer, size, 0);
        pos += size;
        return size;
    }

    void seekRelative(i64 offset) noexcept final
    {
        pos += static_cast<u64>(offset);
    }

    void seekAbsolute(u64 offset) noexcept final
    {
        pos = offset;
    }

    u64 position() noexcept final
    {
        return pos;
    }

    void clearErrors() noexcept final {}

    bool close() noexcept final
    {
        return true;
    }

private:
    /// Allows placing the vtable in only one TU while while all other virtual functions are defined in the class.
    virtual void vtablePlacer();
};

/**
 * @brief A special output stream that simulates an infinite sink for bytes.
 * It only keeps track of the position so that relative and absolute seeking is consistent.
 */
class NullOutputStream final : public OutputStream {
private:
    u64 pos = 0;

public:
    NullOutputStream() noexcept
    {
        this->flags.err = false;
    }

    NullOutputStream(NullOutputStream &&) noexcept = default;
    ~NullOutputStream() noexcept final = default;

    void write(u8) noexcept final
    {
        pos++;
    }

    void write(const u8 *, usize size) noexcept final
    {
        pos += size;
    }

    void seekRelative(i64 offset) noexcept final
    {
        pos += static_cast<u64>(offset);
    }

    void seekAbsolute(u64 offset) noexcept final
    {
        pos = offset;
    }

    u64 position() noexcept final
    {
        return pos;
    }

    void flush() noexcept final {}

    bool close() noexcept final
    {
        return true;
    }

private:
    /// Allows placing the vtable in only one TU while while all other virtual functions are defined in the class.
    virtual void vtablePlacer();
};

// BYTE STREAMS ========================================================================================================

class ByteArrayInputStream final : public InputStream {
private:
    const u8 *data_;
    usize size_;
    usize pos = 0;

public:
    ByteArrayInputStream(const u8 data[], usize size) noexcept : data_{data}, size_{size}
    {
        this->flags.eof = false;
        this->flags.err = false;
    }

    ByteArrayInputStream(ByteArrayInputStream &&) noexcept = default;
    ~ByteArrayInputStream() noexcept final = default;

    explicit ByteArrayInputStream(const ByteArrayOutputStream &outputStream) noexcept;

    u8 read() noexcept final
    {
        if (pos >= size()) {
            this->flags.eof = true;
            return 0;
        }
        return data_[pos++];
    }

    usize read(u8 buffer[], usize size) noexcept final
    {
        if (this->pos >= this->size()) {
            this->flags.eof = true;
            return 0;
        }
        usize readCount = std::min(this->size() - this->pos, size);
        if (readCount != size) {
            this->flags.eof = true;
        }
        std::copy(data_ + pos, data_ + pos + readCount, buffer);
        pos += readCount;
        return readCount;
    }

    void seekRelative(i64 offset) noexcept final
    {
        VXIO_DEBUG_ASSERT_GE(static_cast<i64>(pos) + offset, 0);
        // intentional, works because of two's complement
        seekAbsolute(pos + static_cast<u64>(offset));
    }

    void seekAbsolute(u64 offset) noexcept final
    {
        pos = offset;
        if (pos >= size()) {
            this->flags.eof = true;
        }
    }

    bool close() noexcept final
    {
        return true;
    }

    u64 position() noexcept final
    {
        return pos;
    }

    void clearErrors() noexcept final
    {
        this->flags.eof = false;
        this->flags.err = false;
    }

    size_t size() const noexcept
    {
        return size_;
    }

    const u8 *data() const noexcept
    {
        return data_;
    }

private:
    virtual void vtablePlacer();
};

class ByteArrayOutputStream final : public OutputStream {
public:
    static constexpr usize DEFAULT_INITIAL_SIZE = 8192;

private:
    u64 pos = 0;
    void *sink;  // actually a std::vector<u8>, but not explicitly typed to avoid a <vector> include

public:
    ByteArrayOutputStream(usize initialSize = DEFAULT_INITIAL_SIZE);

    ByteArrayOutputStream(ByteArrayOutputStream &&moveOf) noexcept : pos{moveOf.pos}, sink{moveOf.sink}
    {
        moveOf.sink = nullptr;
    }

    ByteArrayOutputStream(const ByteArrayOutputStream &) = delete;

    ~ByteArrayOutputStream() final;

    void write(u8) final;
    void write(const u8 *, usize size) final;

    void seekRelative(i64 offset) noexcept final
    {
        VXIO_DEBUG_ASSERT_GE(static_cast<i64>(pos) + offset, 0);
        // intentional, works because of two's complement
        pos += static_cast<u64>(offset);
    }

    void seekAbsolute(u64 offset) noexcept final
    {
        pos = offset;
        if (pastEnd()) {
            this->flags.err = true;
        }
    }

    u64 position() noexcept final
    {
        return pos;
    }

    void flush() noexcept final {}

    bool close() noexcept final
    {
        return true;
    }

    void clear() noexcept;

    void reserve(usize size);

    u8 *data() noexcept;

    const u8 *data() const noexcept;

    /**
     * @brief Returns the total number of bytes written to the stream.
     *
     * This method works independently from position().
     * Even when a certain position in the file is seeked, the size remains the same.
     * Otherwise this is equivalent to position().
     *
     * @return the total number of written bytes
     */
    usize size() const noexcept;

private:
    bool atEnd() const noexcept
    {
        return pos == size();
    }
    bool pastEnd() const noexcept
    {
        return pos > size();
    }
};

inline ByteArrayInputStream::ByteArrayInputStream(const ByteArrayOutputStream &outputStream) noexcept
    : ByteArrayInputStream{outputStream.data(), outputStream.size()}
{
}

}  // namespace voxelio

#endif  // VXIO_STREAM_HPP
