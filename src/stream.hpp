#ifndef VXIO_STREAM_HPP
#define VXIO_STREAM_HPP

#include "assert.hpp"
#include "endian.hpp"
#include "types.hpp"

#include <cstddef>
#include <iosfwd>
#include <optional>

namespace voxelio {

static_assert(sizeof(u8) == 1);
static_assert(sizeof(i8) == 1);

/**
 * @brief Java-style implementation of an input stream.
 * In addition to a collection of abstract functions which provide input functionality, this class also contains flags
 * for fast error checking similar to C++ streams.
 * Numerous utility member functions are included in this stream which allow decoding data in various formats.
 */
class InputStream {
protected:
    struct Flags {
        bool eof : 1;
        bool err : 1;
    };

    u8 readBuffer[sizeof(uintmax_t) * 4 - sizeof(Flags)];
    Flags flags;

public:
    // VIRTUAL ----
    // --------------------------------------------------------------------------------------------------------

    InputStream() = default;
    InputStream(const InputStream &) = default;
    InputStream(InputStream &&) = default;
    virtual ~InputStream();

    /**
     * @brief Reads a single byte from the stream.
     * May set eof and err.
     * @return the byte or EOF if extraction failed
     */
    virtual int read() = 0;

    /**
     * @brief Reads maxSize characters into the buffer.
     * May set eof and err.
     * @param buffer the buffer
     * @param maxSize the maximum number of characters to read
     */
    virtual size_t read(u8 buffer[], size_t maxSize) = 0;

    /**
     * @brief Moves the read position to a given absolute position.
     * May set eof and err.
     * @param position the position to move to
     */
    virtual void seekAbsolute(u64 position) = 0;

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

    // DEFAULTS
    // ------------------------------------------------------------------------------------------------------------

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
    virtual size_t read(u8 buffer[], size_t size, u8 delimiter)
    {
        size_t readCount = read(buffer, size);
        if (err()) {
            return 0;
        }

        // At this point we either reached the EOF or read all characters.
        // It doesn't really matter, we need to find the delimiter in the stream either way.
        for (size_t i = 0; i < readCount; ++i) {
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
        constexpr size_t chunkSize = 128;
        const u8 delimiterByte = static_cast<u8>(delimiter);
        size_t totalSize = 0;

        do {
            out.resize(totalSize + chunkSize);
            u8 *stringData = reinterpret_cast<u8 *>(out.data());
            size_t readSize = read(stringData + totalSize, chunkSize, delimiterByte);
            totalSize += readSize;
            if (readSize != chunkSize) {
                out.resize(totalSize);
                return;
            }
        } while (good());
    }

    /**
     * @brief Moves the read position relatively by a given offset.
     * May set eof and err.
     * @param offset the offset from the current read position
     */
    virtual void seekRelative(i64 offset)
    {
        i64 pos = static_cast<i64>(position()) + offset;
        if (not err()) {
            seekAbsolute(static_cast<u64>(pos));
        }
    }

    // FLAG HANDLING
    // -------------------------------------------------------------------------------------------------------

    /**
     * @brief Returns true if the EOF (end of file) has been reached.
     * This flag will only be set when a read PAST the end of a file occurs.
     * When the final byte of a file is read correctly, this flag will not be set.
     * @return true if the EOF has been reached
     */
    bool eof() const
    {
        return flags.eof;
    }

    /**
     * @brief Returns true if a read operation failed.
     * @return true if a read operation failed
     */
    bool err() const
    {
        return flags.err;
    }

    /**
     * @brief Returns true if neither eof nor err are set.
     * @return true if neither eof nor err are set
     */
    bool good() const
    {
        return !eof() && !err();
    }

    // UTILITY
    // -------------------------------------------------------------------------------------------------------------

    /**
     * @brief Reads a number of characters into a string.
     * May set eof and err.
     * @param out the string
     * @param length the number of characters to read
     */
    void readStringTo(std::string &out, size_t length)
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
    std::string readString(size_t length)
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
        return readData<u8, Endian::LITTLE>();
    }

    /**
     * @brief Reads a signed 8-bit integer.
     * May set eof and err.
     * @return the read data
     */
    i8 readI8()
    {
        return readData<i8, Endian::LITTLE>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, int> = 0>
    T readBig()
    {
        return readData<T, Endian::BIG>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, int> = 0>
    T readLittle()
    {
        return readData<T, Endian::LITTLE>();
    }

    /**
     * @brief Reads multiple unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <size_t COUNT, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    void readBig(T t[COUNT])
    {
        return readData<T, Endian::BIG, COUNT>(t);
    }

    /**
     * @brief Reads multiple unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <size_t COUNT, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    void readLittle(T t[COUNT])
    {
        return readData<T, Endian::LITTLE, COUNT>(t);
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
            read(readBuffer, sizeof(T));
            if constexpr (sizeof(T) == 1) {
                return static_cast<T>(readBuffer[0]);
            }
            else {
                return detail::decode_builtin<ENDIAN, T>(readBuffer);
            }
        }
    }

    template <typename T, Endian ENDIAN, size_t COUNT>
    void readDataToBuffer(T t[COUNT], u8 buffer[sizeof(T) * COUNT])
    {
        read(buffer, sizeof(T) * COUNT);

        for (size_t i = 0; i < COUNT; ++i) {
            u8 *insertionPos = buffer + sizeof(T) * i;
            t[i] = detail::decode_builtin<ENDIAN, T>(insertionPos);
        }
    }

    template <typename T, Endian ENDIAN, size_t COUNT>
    void readData(T t[COUNT])
    {
        if constexpr (sizeof(T) == 1) {
            read(reinterpret_cast<u8 *>(t), COUNT);
        }
        else {
            constexpr size_t bufferSize = sizeof(T) * COUNT;
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
    struct Flags {
        bool err : 1;
        bool bin : 1;
    };

    u8 writeBuffer[sizeof(uintmax_t) * 4 - sizeof(Flags)];
    Flags flags;

public:
    // ABSTRACT
    // ------------------------------------------------------------------------------------------------------------

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
    virtual void write(const u8 buffer[], size_t size) = 0;

    /**
     * @brief Writes a C-string to the stream.
     * May set err.
     * @param string the string
     */
    virtual void write(const char *string) = 0;

    /**
     * @brief Moves the write position to a given absolute position.
     * May set err.
     * @param position the position to move to
     */
    virtual void seekAbsolute(u64 position) = 0;

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

    // DEFAULTS
    // ------------------------------------------------------------------------------------------------------------

    /**
     * @brief Moves the write position relatively by a given offset.
     * May set err.
     * @param offset the offset from the current write position
     */
    virtual void seekRelative(i64 offset)
    {
        i64 pos = static_cast<i64>(position()) + offset;
        if (not err()) {
            seekAbsolute(static_cast<u64>(pos));
        }
    }

    // FLAG HANDLING
    // -------------------------------------------------------------------------------------------------------

    /**
     * @brief Returns true if a write operation failed.
     * @return true if a write operation failed
     */
    bool err() const
    {
        return flags.err;
    }

    /**
     * @brief Returns true if no error occured.
     * @return true if no error occured.
     */
    bool good() const
    {
        return not err();
    }

    /**
     * @brief Returns true if the stream was opened in binary mode.
     * @return true if no error occured
     */
    bool bin() const
    {
        return flags.bin;
    }

    // UTILITY
    // -------------------------------------------------------------------------------------------------------------

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
#ifdef _WIN32
        writeString(str + (flags.bin ? "\r\n" : "\n"));
#else
        writeString(str + '\n');
#endif
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
     * @brief Writes an unsigned, big-endian arithmetic type.
     * May set err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, int> = 0>
    void writeBig(T t)
    {
        return writeData<T, Endian::BIG>(t);
    }

    /**
     * @brief Writes an unsigned, big-endian arithmetic type.
     * May set err.
     * @return the read data
     */
    template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, int> = 0>
    void writeLittle(T t)
    {
        return writeData<T, Endian::LITTLE>(t);
    }

    /**
     * @brief Writes multiple unsigned, big-endian arithmetic type.
     * May set err.
     * @param t the buffer for read data
     */
    template <size_t COUNT, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    void writeBig(const T t[COUNT])
    {
        return writeData<T, Endian::BIG, COUNT>(t);
    }

    /**
     * @brief Writes multiple unsigned, big-endian arithmetic type.
     * May set err.
     * @param t the buffer for read data
     */
    template <size_t COUNT, typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    void writeLittle(const T t[COUNT])
    {
        return writeData<T, Endian::LITTLE, COUNT>(t);
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
                detail::encode_builtin<ENDIAN, T>(data, writeBuffer);
                write(writeBuffer, sizeof(T));
            }
        }
    }

    template <typename T, Endian ENDIAN, size_t COUNT>
    void writeDataToBuffer(const T t[COUNT], u8 buffer[sizeof(T) * COUNT])
    {
        for (size_t i = 0; i < COUNT; ++i) {
            u8 *insertionPos = buffer + sizeof(T) * i;
            detail::encode_builtin<ENDIAN, T>(t[i], insertionPos);
        }

        write(buffer, sizeof(T) * COUNT);
    }

    template <typename T, Endian ENDIAN, size_t COUNT>
    void writeData(const T data[COUNT])
    {
        if constexpr (sizeof(T) == 1) {
            write(reinterpret_cast<const u8 *>(data), COUNT);
        }
        else {
            constexpr size_t bufferSize = sizeof(T) * COUNT;
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

/**
 * @brief The mode in which files are opened by FileInputStream and FileOutputStream.
 */
struct OpenMode {
    enum Value : unsigned {
        /** Indicates that the stream should be opened for reading. Is always set for FileInputStream. */
        READ = 1 << 0,
        /** Indicates that the stream should be opened for writing. Is always set for FileOutputStream. */
        WRITE = 1 << 1,
        /** Indicates that when writing, new bytes should be appended at the end of the file. */
        APPEND = 1 << 2,
        /** Indicates that the stream is to be opened in binary mode.
         * This means that CRLF endings are not translated automatically by the OS. */
        BINARY = 1 << 3,
        /** Indicates that new files should not be overwritten by opening a FileOutputStream. */
        PRESERVE = 1 << 4
    };
};

// NULL STREAMS ========================================================================================================

/**
 * @brief A special input stream that simulates an infinite file filled with zero-bytes.
 * It only keeps track of the position so that relative and absolute seeking is consistent.
 */
class NullInputStream : public InputStream {
private:
    u64 pos = 0;

    NullInputStream() noexcept;

public:
    NullInputStream(NullInputStream &&) = default;
    ~NullInputStream() final = default;

    int read() final
    {
        pos++;
        return 0;
    }
    size_t read(u8 buffer[], size_t size) final
    {
        std::fill_n(buffer, size, 0);
        pos += size;
        return size;
    }

    void seekRelative(i64 offset) final
    {
        pos += static_cast<u64>(offset);
    }

    void seekAbsolute(u64 offset) final
    {
        pos = offset;
    }

    u64 position() final
    {
        return pos;
    }
    void clearErrors() final {}
};

/**
 * @brief A special output stream that simulates an infinite sink for bytes.
 * It only keeps track of the position so that relative and absolute seeking is consistent.
 */
class NullOutputStream : public OutputStream {
private:
    u64 pos = 0;

    NullOutputStream() noexcept;

public:
    NullOutputStream(NullOutputStream &&) = default;
    ~NullOutputStream() final = default;

    void write(u8) final
    {
        pos++;
    }

    void write(const u8 *, size_t size) final
    {
        pos += size;
    }

    void write(const char *string) final
    {
        pos += std::strlen(string);
    }

    void seekRelative(i64 offset) final
    {
        pos += static_cast<u64>(offset);
    }

    void seekAbsolute(u64 offset) final
    {
        pos = offset;
    }

    u64 position() final
    {
        return pos;
    }
    void flush() final {}
};

// FILE STREAMS ========================================================================================================

/**
 * @brief Implementation of InputStream using the C-File-API.
 */
class FileInputStream : public InputStream {
public:
    static std::optional<FileInputStream> open(const char *path, unsigned mode = OpenMode::READ);

    static std::optional<FileInputStream> open(const std::string &path, unsigned mode = OpenMode::READ)
    {
        return open(path.c_str(), mode);
    }

private:
    cfile file;

    FileInputStream(cfile file) : file{file}
    {
        VXIO_DEBUG_ASSERT_NOTNULL(file);
    }

public:
    FileInputStream(FileInputStream &&);
    ~FileInputStream() final;

    int read() final;
    size_t read(u8 buffer[], size_t size) final;
    void seekRelative(i64 offset) final;
    void seekAbsolute(u64 offset) final;
    u64 position() final;
    void clearErrors() final;

private:
    void updateErrorFlags();
};

/**
 * @brief Implementation of OutputStream using the C-File-API.
 */
class FileOutputStream : public OutputStream {
public:
    static std::optional<FileOutputStream> open(const char *path, unsigned mode = OpenMode::WRITE);

    static std::optional<FileOutputStream> open(const std::string &path, unsigned mode = OpenMode::WRITE)
    {
        return open(path.c_str(), mode);
    }

private:
    cfile file;

    FileOutputStream(cfile file) : file{file}
    {
        VXIO_DEBUG_ASSERT_NOTNULL(file);
    }

public:
    FileOutputStream(FileOutputStream &&);
    ~FileOutputStream() final;

    void write(u8 byte) final;
    void write(const u8 *buffer, size_t size) final;
    void write(const char *string) final;
    void seekRelative(i64 offset) final;
    void seekAbsolute(u64 offset) final;
    u64 position() final;
    void flush() final;

private:
    void updateErrorFlags();
};

inline std::optional<FileInputStream> openForRead(const char *path, unsigned mode = OpenMode::READ)
{
    return FileInputStream::open(path, mode);
}

inline std::optional<FileInputStream> openForRead(const std::string &path, unsigned mode = OpenMode::READ)
{
    return FileInputStream::open(path, mode);
}

inline std::optional<FileOutputStream> openForWrite(const char *path, unsigned mode = OpenMode::WRITE)
{
    return FileOutputStream::open(path, mode);
}

inline std::optional<FileOutputStream> openForWrite(const std::string &path, unsigned mode = OpenMode::WRITE)
{
    return FileOutputStream::open(path, mode);
}

// STANDARD STREAM WRAPPERS ============================================================================================

/**
 * @brief Adapter which allows using std::istream as a voxelio::InputStream.
 */
class StdInputStream : public InputStream {
private:
    std::istream *stream;

public:
    explicit StdInputStream(std::istream &stream);

    int read() final;
    size_t read(u8 *buffer, size_t size) final;
    void readStringToUntil(std::string &out, char delimiter) final;
    void seekRelative(i64 offset) final;
    void seekAbsolute(u64 offset) final;
    u64 position() final;
    void clearErrors() final;

private:
    void updateErrorFlags();
};

/**
 * @brief Adapter which allows using std::ostream as a voxelio::OutputStream.
 */
class StdOutputStream : public OutputStream {
private:
    std::ostream *stream;

public:
    explicit StdOutputStream(std::ostream &stream);

    void write(u8 byte) final;
    void write(const u8 *buffer, size_t size) final;
    void write(const char *string) final;
    void seekRelative(i64 offset) final;
    void seekAbsolute(u64 offset) final;
    u64 position() final;
    void flush() final;

private:
    void updateErrorFlags();
};

}  // namespace voxelio

#endif  // VXIO_STREAM_HPP
