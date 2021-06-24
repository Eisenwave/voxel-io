#include "voxelio/stream.hpp"

#include "voxelio/fstream.hpp"
#include "voxelio/log.hpp"

#include <istream>
#include <ostream>
#include <vector>

namespace voxelio {

InputStream::~InputStream() = default;

OutputStream::~OutputStream() = default;

// =====================================================================================================================

void NullInputStream::vtablePlacer() {}

void NullOutputStream::vtablePlacer() {}

void ByteArrayInputStream::vtablePlacer() {}

// =====================================================================================================================

ByteArrayOutputStream::ByteArrayOutputStream(usize initialSize) : sink{new std::vector<u8>}
{
    reserve(initialSize);
    this->flags.err = false;
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
    delete static_cast<std::vector<u8> *>(sink);
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

void ByteArrayOutputStream::write(const u8 data[], usize length)
{
    if (pastEnd()) {
        this->flags.err = true;
        return;
    }

    usize overwriteLength = std::min(size() - pos, length);
    usize pushLength = length - overwriteLength;

    auto overwriteTarget = static_cast<std::vector<u8> *>(sink)->begin() + static_cast<std::ptrdiff_t>(pos);
    std::copy(data, data + overwriteLength, overwriteTarget);

    for (usize i = 0; i < pushLength; ++i) {
        static_cast<std::vector<u8> *>(sink)->push_back(data[overwriteLength + i]);
    }

    pos += length;
}

void ByteArrayOutputStream::clear() noexcept
{
    static_cast<std::vector<u8> *>(sink)->clear();
    this->pos = 0;
    this->flags.err = false;
}

void ByteArrayOutputStream::reserve(usize size)
{
    static_cast<std::vector<u8> *>(sink)->reserve(size);
}

u8 *ByteArrayOutputStream::data() noexcept
{
    return static_cast<std::vector<u8> *>(sink)->data();
}

const u8 *ByteArrayOutputStream::data() const noexcept
{
    return static_cast<const std::vector<u8> *>(sink)->data();
}

usize ByteArrayOutputStream::size() const noexcept
{
    return static_cast<const std::vector<u8> *>(sink)->size();
}

// =====================================================================================================================

u8 FileInputStream::read() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    int rawResult = std::fgetc(file);
    if (rawResult == EOF) {
        updateErrorFlags();
        return 0;
    }
    return static_cast<u8>(rawResult);
}

usize FileInputStream::read(u8 buffer[], usize size) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (size == 0) {
        return 0;
    }
    usize count = std::fread(reinterpret_cast<char *>(buffer), 1, size, file);
    if (count != size) {
        updateErrorFlags();
    }
    return count;
}

void FileInputStream::seekRelative(i64 offset) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fseek(file, offset, SEEK_CUR) != 0) {
        updateErrorFlags();
    }
}

void FileInputStream::seekAbsolute(u64 offset) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fseek(file, static_cast<long>(offset), SEEK_SET) != 0) {
        updateErrorFlags();
    }
}

u64 FileInputStream::position() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    long rawResult = std::ftell(file);
    if (rawResult < 0) {
        flags.err = true;
        return 0;
    }
    return static_cast<u64>(rawResult);
}

void FileInputStream::clearErrors() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    std::clearerr(file);
    flags.eof = false;
    flags.err = false;
}

bool FileInputStream::close() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    bool err = std::fclose(file) != 0;
    flags.err |= err;
    file = nullptr;
    return err;
}

void FileInputStream::updateErrorFlags() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    flags.eof = std::feof(file);
    flags.err = std::ferror(file);
    VXIO_LOG(SUPERSPAM, "Updated error flags");
}

// =====================================================================================================================

void FileOutputStream::write(u8 byte) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fputc(byte, file) == EOF) {
        flags.err = true;
    }
}

void FileOutputStream::write(const u8 *buffer, usize size) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fwrite(reinterpret_cast<const char *>(buffer), 1, size, file) != size) {
        flags.err = true;
    }
}

void FileOutputStream::write(const char *string) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fputs(string, file) == EOF) {
        flags.err = true;
    }
}

void FileOutputStream::seekRelative(i64 offset) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fseek(file, offset, SEEK_CUR) != 0) {
        flags.err = true;
    }
}

void FileOutputStream::seekAbsolute(u64 offset) noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fseek(file, static_cast<long>(offset), SEEK_SET) != 0) {
        flags.err = true;
    }
}

u64 FileOutputStream::position() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    long rawResult = std::ftell(file);
    if (rawResult < 0) {
        flags.err = true;
        return 0;
    }
    return static_cast<u64>(rawResult);
}

void FileOutputStream::flush() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    if (std::fflush(file) != 0) {
        flags.err = true;
    }
}

bool FileOutputStream::close() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    bool err = std::fclose(file) != 0;
    flags.err |= err;
    file = nullptr;
    return err;
}

void FileOutputStream::updateErrorFlags() noexcept
{
    VXIO_ASSERT_NOTNULL(file);
    flags.err = std::ferror(file);
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

usize StdInputStream::read(u8 buffer[], usize size)
{
    static_assert(sizeof(u8) == sizeof(std::istream::char_type));
    stream->read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(size));
    updateErrorFlags();
    return static_cast<usize>(stream->gcount());
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

bool StdInputStream::close()
{
    return stream->rdbuf(nullptr) != nullptr;
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

void StdOutputStream::write(const u8 *buffer, usize size)
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

bool StdOutputStream::close()
{
    return stream->rdbuf(nullptr) != nullptr;
}

void StdOutputStream::updateErrorFlags()
{
    flags.err = stream->bad();
}

}  // namespace voxelio
