#ifndef VXIO_FSTREAM_HPP
#define VXIO_FSTREAM_HPP

#include "cstringview.hpp"
#include "log.hpp"
#include "stream.hpp"

namespace voxelio {

/**
 * @brief The mode in which files are opened by FileInputStream and FileOutputStream.
 */
enum class OpenMode : unsigned {
    /** No flags. For internal use only. */
    NONE = 0,
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
    PRESERVE = 1 << 4,
    /** Indicates that the file should be opened with no buffer for I/O operations. */
    UNBUFFERED = 1 << 5
};

constexpr OpenMode operator~(OpenMode x) noexcept
{
    return static_cast<OpenMode>(~toUnderlying(x));
}

constexpr OpenMode operator|(OpenMode x, OpenMode y) noexcept
{
    return static_cast<OpenMode>(toUnderlying(x) | toUnderlying(y));
}

constexpr OpenMode operator&(OpenMode x, OpenMode y) noexcept
{
    return static_cast<OpenMode>(toUnderlying(x) & toUnderlying(y));
}

constexpr OpenMode operator^(OpenMode x, OpenMode y) noexcept
{
    return static_cast<OpenMode>(toUnderlying(x) ^ toUnderlying(y));
}

constexpr OpenMode &operator|=(OpenMode &x, OpenMode y) noexcept
{
    return x = x | y;
}

constexpr OpenMode &operator&=(OpenMode &x, OpenMode y) noexcept
{
    return x = x & y;
}

constexpr OpenMode &operator^=(OpenMode &x, OpenMode y) noexcept
{
    return x = x ^ y;
}

// FILE STREAMS ========================================================================================================

namespace detail {

constexpr const char *openModeStringForReadOf(OpenMode mode)
{
    // toUnderlying is used to suppress -Wswitch here
    mode &= OpenMode::WRITE | OpenMode::BINARY;
    switch (toUnderlying(mode)) {
    case toUnderlying(OpenMode::NONE): return "r";
    case toUnderlying(OpenMode::WRITE): return "r+";
    case toUnderlying(OpenMode::BINARY): return "rb";
    case toUnderlying(OpenMode::WRITE | OpenMode::BINARY): return "r+b";
    default: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

inline std::string openModeStringForWriteOf(OpenMode mode)
{
    std::string modeStr = (mode & OpenMode::READ) != OpenMode::NONE ? "w+" : "w";
    if ((mode & OpenMode::APPEND) != OpenMode::NONE) {
        modeStr += 'a';
    }
    if ((mode & OpenMode::PRESERVE) != OpenMode::NONE) {
        modeStr += 'x';
    }
    if ((mode & OpenMode::BINARY) != OpenMode::NONE) {
        modeStr += 'b';
    }
    return modeStr;
}

}  // namespace detail

/**
 * @brief Implementation of InputStream using the C-File-API.
 */
class FileInputStream final : public InputStream {
private:
    std::FILE *file;

public:
    explicit FileInputStream(std::FILE *file) noexcept : file{file}
    {
        if constexpr (build::DEBUG) {
            std::FILE *stdoutput = stdout;
            VXIO_ASSERT_NE(file, stdoutput);
        }
        flags.err = file == nullptr || std::ferror(file);
        flags.eof = file != nullptr && std::feof(file);
    }

    FileInputStream(CStringView path, OpenMode mode) noexcept
        : file{std::fopen(path, detail::openModeStringForReadOf(mode))}
    {
        this->flags.err = file == nullptr;
        this->flags.eof = false;

        if (good() && (mode & OpenMode::UNBUFFERED) != OpenMode::NONE) {
            std::setbuf(file, nullptr);
        }
    }

    FileInputStream(FileInputStream &&moveOf) noexcept : InputStream{std::move(moveOf)}, file{moveOf.file}
    {
        VXIO_DEBUG_ASSERT_NE(this, &moveOf);
        moveOf.file = nullptr;
    }

    ~FileInputStream() noexcept final
    {
        if (file != nullptr && std::fclose(file) != 0) {
            VXIO_LOG(ERROR, "Failed to close file: " + stringify(file));
            VXIO_DEBUG_ASSERT_UNREACHABLE();
        }
    }

    FileInputStream &operator=(FileInputStream &&moveOf) noexcept
    {
        VXIO_DEBUG_ASSERT_NE(this, &moveOf);
        this->flags = moveOf.flags;
        this->file = moveOf.file;
        moveOf.file = nullptr;
        return *this;
    }

    u8 read() noexcept final;
    usize read(u8 buffer[], usize size) noexcept final;
    void seekRelative(i64 offset) noexcept final;
    void seekAbsolute(u64 offset) noexcept final;
    u64 position() noexcept final;
    void clearErrors() noexcept final;

private:
    void updateErrorFlags() noexcept;
};

/**
 * @brief Implementation of OutputStream using the C-File-API.
 */
class FileOutputStream final : public OutputStream {
private:
    std::FILE *file;

public:
    explicit FileOutputStream(std::FILE *file) noexcept : file{file}
    {
        if constexpr (build::DEBUG) {
            std::FILE *stdinput = stdin;
            VXIO_ASSERT_NE(file, stdinput);
        }
        flags.err = file == nullptr || std::ferror(file);
    }

    FileOutputStream(CStringView path, OpenMode mode) noexcept
        : file{std::fopen(path, detail::openModeStringForWriteOf(mode).c_str())}
    {
        this->flags.err = file == nullptr;

        if (good() && (mode & OpenMode::UNBUFFERED) != OpenMode::NONE) {
            std::setbuf(file, nullptr);
        }
    }

    FileOutputStream(FileOutputStream &&moveOf) noexcept : OutputStream{std::move(moveOf)}, file{moveOf.file}
    {
        VXIO_DEBUG_ASSERT_NE(this, &moveOf);
        moveOf.file = nullptr;
    }

    ~FileOutputStream() noexcept final
    {
        if (file != nullptr && std::fclose(file) != 0) {
            VXIO_LOG(ERROR, "Failed to close file: " + stringify(file));
            VXIO_DEBUG_ASSERT_UNREACHABLE();
        }
    }

    FileOutputStream &operator=(FileOutputStream &&moveOf) noexcept
    {
        VXIO_DEBUG_ASSERT_NE(this, &moveOf);
        this->flags = moveOf.flags;
        this->file = moveOf.file;
        moveOf.file = nullptr;
        return *this;
    }

    void write(u8 byte) noexcept final;
    void write(const u8 *buffer, usize size) noexcept final;
    void write(const char *string) noexcept final;
    void seekRelative(i64 offset) noexcept final;
    void seekAbsolute(u64 offset) noexcept final;
    u64 position() noexcept final;
    void flush() noexcept final;

private:
    void updateErrorFlags() noexcept;
};

// STANDARD STREAM WRAPPERS ============================================================================================

/**
 * @brief Adapter which allows using std::istream as a voxelio::InputStream.
 */
class StdInputStream final : public InputStream {
private:
    std::istream *stream;

public:
    explicit StdInputStream(std::istream &stream);

    u8 read() final;
    usize read(u8 *buffer, usize size) final;
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
class StdOutputStream final : public OutputStream {
private:
    std::ostream *stream;

public:
    explicit StdOutputStream(std::ostream &stream);

    void write(u8 byte) final;
    void write(const u8 *buffer, usize size) final;
    void write(const char *string) final;
    void seekRelative(i64 offset) final;
    void seekAbsolute(u64 offset) final;
    u64 position() final;
    void flush() final;

private:
    void updateErrorFlags();
};

}  // namespace voxelio

#endif  // VXIO_FSTREAM_HPP
