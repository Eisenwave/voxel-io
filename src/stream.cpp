#include "voxelio/stream.hpp"

#include "voxelio/log.hpp"

#include <istream>
#include <ostream>
#include <vector>

namespace voxelio {

InputStream::~InputStream() = default;

OutputStream::~OutputStream() = default;

static std::string openModeStringForReadOf(unsigned mode, bool &outBinary)
{
    std::string modeStr = (mode & OpenMode::WRITE) ? "r+" : "r";
    if ((outBinary = (mode & OpenMode::BINARY))) {
        modeStr += 'b';
    }
    return modeStr;
}

static std::string openModeStringForWriteOf(unsigned mode, bool &outBinary)
{
    std::string modeStr = (mode & OpenMode::READ) ? "w+" : "w";
    if (mode & OpenMode::APPEND) {
        modeStr += 'a';
    }
    if (mode & OpenMode::PRESERVE) {
        modeStr += 'x';
    }
    if ((outBinary = (mode & OpenMode::BINARY))) {
        modeStr += 'b';
    }
    return modeStr;
}

// =====================================================================================================================

NullInputStream::NullInputStream() noexcept
{
    this->flags.eof = false;
    this->flags.err = false;
}

NullOutputStream::NullOutputStream() noexcept
{
    this->flags.err = false;
}

// =====================================================================================================================

ByteArrayInputStream::ByteArrayInputStream(const u8 data[], size_t size) noexcept : data_{data}, size_{size}
{
    this->flags.eof = false;
    this->flags.err = false;
}

u8 ByteArrayInputStream::read()
{
    if (pos >= size()) {
        this->flags.eof = true;
        return 0;
    }
    return data_[pos++];
}

size_t ByteArrayInputStream::read(u8 buffer[], size_t size)
{
    if (this->pos >= this->size()) {
        this->flags.eof = true;
        return 0;
    }
    size_t readCount = std::min(this->size() - this->pos, size);
    if (readCount != size) {
        this->flags.eof = true;
    }
    std::copy(data_ + pos, data_ + pos + readCount, buffer);
    pos += readCount;
    return readCount;
}

// =====================================================================================================================

ByteArrayOutputStream::ByteArrayOutputStream(size_t initialSize) noexcept : sink{new std::vector<u8>}
{
    static_cast<std::vector<u8> *>(sink)->reserve(initialSize);
    this->flags.err = false;
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
    if (sink != nullptr) {
        delete static_cast<std::vector<u8> *>(sink);
    }
}

void ByteArrayOutputStream::write(u8 byte)
{
    if (pastEnd()) {
        this->flags.err = true;
    }
    else if (atEnd()) {
        static_cast<std::vector<u8> *>(sink)->push_back(byte);
        pos++;
    }
    else {
        static_cast<std::vector<u8> *>(sink)->operator[](pos++) = byte;
    }
}

void ByteArrayOutputStream::write(const u8 data[], size_t length)
{
    if (pastEnd()) {
        this->flags.err = true;
        return;
    }

    size_t overwriteLength = std::min(size() - pos, length);
    size_t pushLength = length - overwriteLength;

    auto overwriteTarget = static_cast<std::vector<u8> *>(sink)->begin() + static_cast<ptrdiff_t>(pos);
    std::copy(data, data + overwriteLength, overwriteTarget);

    for (size_t i = 0; i < pushLength; ++i) {
        static_cast<std::vector<u8> *>(sink)->push_back(data[overwriteLength + i]);
    }

    pos += length;
}

void ByteArrayOutputStream::clear()
{
    static_cast<std::vector<u8> *>(sink)->clear();
    this->pos = 0;
    this->flags.err = false;
}

void ByteArrayOutputStream::reserve(size_t size)
{
    static_cast<std::vector<u8> *>(sink)->reserve(size);
}

u8 *ByteArrayOutputStream::data()
{
    return static_cast<std::vector<u8> *>(sink)->data();
}

const u8 *ByteArrayOutputStream::data() const
{
    return static_cast<const std::vector<u8> *>(sink)->data();
}

size_t ByteArrayOutputStream::size() const
{
    return static_cast<const std::vector<u8> *>(sink)->size();
}

// =====================================================================================================================

std::optional<FileInputStream> FileInputStream::open(const char *path, unsigned mode)
{
    bool bin;
    std::string modeStr = openModeStringForReadOf(mode, bin);
    cfile file = std::fopen(path, modeStr.c_str());

    if (file == nullptr) {
        return std::nullopt;
    }

    FileInputStream result{file};
    result.flags.eof = false;
    result.flags.err = false;

    return result;
}

FileInputStream::FileInputStream(FileInputStream &&moveOf) : InputStream{std::move(moveOf)}, file{moveOf.file}
{
    moveOf.file = nullptr;
}

FileInputStream::~FileInputStream()
{
    if (file != nullptr && std::fclose(file) != 0) {
        VXIO_LOG(ERROR, "Failed to close file: " + stringify(file));
        VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

FileInputStream &FileInputStream::operator=(FileInputStream &&moveOf)
{
    this->flags = moveOf.flags;
    this->file = moveOf.file;
    moveOf.file = nullptr;
    return *this;
}

u8 FileInputStream::read()
{
    int rawResult = std::fgetc(file);
    if (rawResult == EOF) {
        updateErrorFlags();
        return 0;
    }
    return static_cast<u8>(rawResult);
}

size_t FileInputStream::read(u8 buffer[], size_t size)
{
    if (size == 0) {
        return 0;
    }
    size_t count = std::fread(reinterpret_cast<char *>(buffer), 1, size, file);
    if (count != size) {
        updateErrorFlags();
    }
    return count;
}

void FileInputStream::seekRelative(i64 offset)
{
    if (std::fseek(file, offset, SEEK_CUR) != 0) {
        updateErrorFlags();
    }
}

void FileInputStream::seekAbsolute(u64 offset)
{
    if (std::fseek(file, static_cast<long>(offset), SEEK_SET) != 0) {
        updateErrorFlags();
    }
}

u64 FileInputStream::position()
{
    long rawResult = std::ftell(file);
    if (rawResult < 0) {
        flags.err = true;
        return 0;
    }
    return static_cast<u64>(rawResult);
}

void FileInputStream::clearErrors()
{
    std::clearerr(file);
    flags.eof = false;
    flags.err = false;
}

void FileInputStream::updateErrorFlags()
{
    flags.eof = std::feof(file);
    flags.err = std::ferror(file);
    VXIO_LOG(SUPERSPAM, "Updated error flags");
}

// =====================================================================================================================
std::optional<FileOutputStream> FileOutputStream::open(const char *path, unsigned mode)
{
    bool bin;
    std::string modeStr = openModeStringForWriteOf(mode, bin);
    cfile file = std::fopen(path, modeStr.c_str());

    if (file == nullptr) {
        return std::nullopt;
    }

    return FileOutputStream{file};
}

FileOutputStream::FileOutputStream(FileOutputStream &&moveOf) : OutputStream{std::move(moveOf)}, file{moveOf.file}
{
    moveOf.file = nullptr;
}

FileOutputStream::~FileOutputStream()
{
    if (file != nullptr && std::fclose(file) != 0) {
        VXIO_LOG(ERROR, "Failed to close file: " + stringify(file));
        VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

FileOutputStream &FileOutputStream::operator=(FileOutputStream &&moveOf)
{
    this->flags = moveOf.flags;
    this->file = moveOf.file;
    moveOf.file = nullptr;
    return *this;
}

void FileOutputStream::write(u8 byte)
{
    if (std::fputc(byte, file) == EOF) {
        flags.err = true;
    }
}

void FileOutputStream::write(const u8 *buffer, size_t size)
{
    if (std::fwrite(reinterpret_cast<const char *>(buffer), 1, size, file) != size) {
        flags.err = true;
    }
}

void FileOutputStream::write(const char *string)
{
    if (std::fputs(string, file) == EOF) {
        flags.err = true;
    }
}

void FileOutputStream::seekRelative(i64 offset)
{
    if (std::fseek(file, offset, SEEK_CUR) != 0) {
        flags.err = true;
    }
}

void FileOutputStream::seekAbsolute(u64 offset)
{
    if (std::fseek(file, static_cast<long>(offset), SEEK_SET) != 0) {
        flags.err = true;
    }
}

u64 FileOutputStream::position()
{
    long rawResult = std::ftell(file);
    if (rawResult < 0) {
        flags.err = true;
        return 0;
    }
    return static_cast<u64>(rawResult);
}

void FileOutputStream::flush()
{
    if (std::fflush(file) != 0) {
        flags.err = true;
    }
}

// =====================================================================================================================

StdInputStream::StdInputStream(std::istream &stream) : stream{&stream}
{
    flags.err = this->stream->bad();
    flags.eof = false;
}

u8 StdInputStream::read()
{
    std::istream::int_type rawResult = stream->get();
    updateErrorFlags();
    return static_cast<u8>(rawResult);
}

size_t StdInputStream::read(u8 buffer[], size_t size)
{
    static_assert(sizeof(u8) == sizeof(std::istream::char_type));
    stream->read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(size));
    updateErrorFlags();
    return static_cast<size_t>(stream->gcount());
}

void StdInputStream::readStringToUntil(std::string &out, char delimiter)
{
    std::getline(*stream, out, delimiter);
    updateErrorFlags();
}

void StdInputStream::seekRelative(i64 offset)
{
    stream->seekg(offset, std::ios::cur);
    flags.eof = stream->eof();
    flags.err = stream->fail();
}

void StdInputStream::seekAbsolute(u64 offset)
{
    stream->seekg(static_cast<std::streamsize>(offset));
    flags.eof = stream->eof();
    flags.err = stream->fail();
}

u64 StdInputStream::position()
{
    u64 result = static_cast<u64>(stream->tellg());
    updateErrorFlags();
    return result;
}

void StdInputStream::clearErrors()
{
    stream->clear();
    flags.eof = false;
    flags.err = false;
}

void StdInputStream::updateErrorFlags()
{
    flags.eof = stream->eof();
    flags.err = stream->bad();
}

// =====================================================================================================================

StdOutputStream::StdOutputStream(std::ostream &stream) : stream{&stream}
{
    flags.err = this->stream->bad();
}

void StdOutputStream::write(u8 byte)
{
    stream->put(static_cast<std::ostream::char_type>(byte));
    updateErrorFlags();
}

void StdOutputStream::write(const u8 *buffer, size_t size)
{
    stream->write(reinterpret_cast<const char *>(buffer), static_cast<std::streamsize>(size));
    updateErrorFlags();
}

void StdOutputStream::write(const char *string)
{
    *stream << string;
    updateErrorFlags();
}

void StdOutputStream::seekRelative(i64 offset)
{
    stream->seekp(static_cast<std::streamsize>(offset), std::ios::cur);
    updateErrorFlags();
}

void StdOutputStream::seekAbsolute(u64 offset)
{
    stream->seekp(static_cast<std::streamsize>(offset));
    updateErrorFlags();
}

u64 StdOutputStream::position()
{
    u64 result = static_cast<u64>(stream->tellp());
    updateErrorFlags();
    return result;
}

void StdOutputStream::flush()
{
    stream->flush();
    updateErrorFlags();
}

void StdOutputStream::updateErrorFlags()
{
    flags.err = stream->bad();
}

}  // namespace voxelio
