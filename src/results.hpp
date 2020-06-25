#ifndef VXIO_RESULTCODE_HPP
#define VXIO_RESULTCODE_HPP

#include <array>
#include <optional>

namespace voxelio {

enum class ResultCode : unsigned char {
    // GOOD RESULTS (0x00..0x3f)

    /** Generic good result. */
    OK = 0x00,
    /** Returned after a successful initialization of the reader or writer. */
    OK_INITIALIZED = 0x01,

    // GOOD READ RESULTS
    /** Generic good read result. */
    READ_OK = 0x10,
    /** Some voxels were read but reading had to be interrupted. */
    READ_BUFFER_FULL = 0x11,
    /** Some amount of voxels was read, but the end of the object has been reached. */
    READ_OBJECT_END = 0x12,
    /** Some amount of voxels was read, but the end of all data was reached. (expected EOF) */
    READ_END = 0x1f,

    // GOOD WRITE RESULTS
    /** Generic good write result. */
    WRITE_OK = 0x20,
    /** A buffer needs to be filled with more voxels before the results get written through. */
    WRITE_BUFFER_UNDERFULL = 0x21,
    /** The end of the data structure has been reached. */
    WRITE_OBJECT_END = 0x22,
    /** The end of all data structures has been reached, no more voxels can be written into the file. */
    WRITE_END = 0x2f,

    // WARNINGS (0x40..0x7f)

    /** Generic warning. */
    WARNING = 0x40,
    /** Generic no-operation. */
    WARNING_NOP = 0x41,
    /** The input results in a nop. Example: Empty buffer was provided when reading. */
    WARNING_INPUT_NOP = 0x42,
    /** Init() was called more than one time. */
    WARNING_DOUBLE_INIT = 0x43,

    // ERRORS (0x80..0xff)

    /** Generic error */
    ERROR = 0x80,

    // USER ERRORS (0x90..0xaf)

    /** An error caused by the user of the reader or writer. */
    USER_ERROR = 0x90,
    /** Generic missing information error. */
    USER_ERROR_MISSING_INFO = 0x91,
    /** The format requires a palette to be used but the user provided none. */
    USER_ERROR_MISSING_PALETTE = 0x92,
    /** The format requires boundaries to be set but the user set none. */
    USER_ERROR_MISSING_BOUNDARIES = 0x93,
    /** An invalid data format was chosen. */
    USER_ERROR_INVALID_FORMAT = 0x94,
    /** An invalid color format was chosen. */
    USER_ERROR_INVALID_COLOR_FORMAT = 0x95,

    // READ ERRORS (0xa0..0xcf)

    /** Generic read error. */
    READ_ERROR = 0xa0,
    /** While reading voxels, the end of file (EOF) was reached unexpectedly. */
    READ_ERROR_UNEXPECTED_EOF = 0xa1,
    /** While reading voxels, an I/O error was produced. (missing file permissions, hardware failure, etc.) */
    READ_ERROR_IO_FAIL = 0xa2,
    /** An illegal character was read. This may be caused by non-ASCII characters in formats like STL. */
    READ_ERROR_ILLEGAL_CHARACTER = 0xa3,

    // SYNTACTICAL READ ERROS (0xb0..0xbf)

    /** Generic syntactical error. */
    READ_ERROR_SYNTACTICAL = 0xa0,
    /** While reading voxels, an error was found in the data. (corrupted file, unsupported version, etc.) */
    READ_ERROR_PARSE_FAIL = 0xa3,
    /** The magic bytes at the beginning of the file didn't match the expected bytes. */
    READ_ERROR_UNEXPECTED_MAGIC = 0xa4,
    /** While parsing, an unexpected symbol such as an invalid enum value or a mismatched bracket etc. was found. */
    READ_ERROR_UNEXPECTED_SYMBOL = 0xa5,
    /** The version of this file is unknown. */
    READ_ERROR_UNKNOWN_VERSION = 0xa6,
    /** A feature such as an extension of this file is unknown. */
    READ_ERROR_UNKNOWN_FEATURE = 0xa7,
    /** An enumeration value was found which is not in the set of recognized values. (unknown color format, etc.) */
    READ_ERROR_CORRUPTED_ENUM = 0xa8,
    /** A boolean value (0/false/1/true) contains other values. (commonly found when u8 is used for boolean storage) */
    READ_ERROR_CORRUPTED_BOOL = 0xa9,
    /** Missing necessary information like dim in BINVOX headers, a necessary key in a JSON object, etc. */
    READ_ERROR_MISSING_DATA = 0xaa,
    /** A JSON list, a dynamically sized list or other list-like structure has the wrong length. */
    READ_ERROR_WRONG_LIST_LENGTH = 0xab,
    /** A field which is unique was present multiple times in the file. */
    READ_ERROR_DUPLICATE_DATA = 0xac,
    /** The root of the file, such as a JSON-object or a VOX-MAIN-Chunk is present multiple times. */
    READ_ERROR_MULTIPLE_ROOTS = 0xad,
    /** The read data has a length which violates specifications, such as an array that exceeds maximum sizes. */
    READ_ERROR_ILLEGAL_DATA_LENGTH = 0xae,
    /** A read string has a length which is insufficient. */
    READ_ERROR_STRING_TOO_SHORT = 0xaf,
    /** A constant such as a fixed reserved value, checksum, etc. didn't match. */
    READ_ERROR_INVALID_CONSTANT = 0xb0,
    /** A checksum such as a hash didn't match the content. */
    READ_ERROR_INVALID_CHECKSUM = 0xb1,
    /** Parsing text to an integer, boolean, or float failed. */
    READ_ERROR_TEXT_DATA_PARSE_FAIL = 0xb2,

    // SEMANTICAL READ ERROS (0xc0..0xcf)
    /** Generic semantical write error. */
    READ_ERROR_SEMANTICAL = 0xc0,
    /** Value outside of permitted range */
    READ_ERROR_VALUE_OUT_OF_BOUNDS = 0xc1,
    /** The version of this file is not supported, but it is a known version. */
    READ_ERROR_UNSUPPORTED_VERSION = 0xc2,
    /** A feature such as an extension of this file is not supported, but it is a known feature. */
    READ_ERROR_UNSUPPORTED_FEATURE = 0xc3,

    // WRITE ERROS (0xd0..0xef)

    /** Generic write error. */
    WRITE_ERROR = 0xd0,
    /** I/O error while writing. */
    WRITE_ERROR_IO_FAIL = 0xd2,
    /** An attempt was made to write data outside accepted boundaries. */
    WRITE_ERROR_OUT_OF_BOUNDS = 0xd3,
    /** An attempt was made to write a position outside of accepted boundaries. */
    WRITE_ERROR_POSITION_OUT_OF_BOUNDS = 0xd4,
    /** An attempt was made to write an index which is outside accepted boundaries. */
    WRITE_ERROR_INDEX_OUT_OF_BOUNDS = 0xd5,
    /** An output format was chosen which can not be written. */
    WRITE_ERROR_UNSUPPORTED_FORMAT = 0xd6,

    // INTERNAL ERRORS (0xf0..0xff)
    /** Generic internal error. */
    INTERNAL_ERROR = 0xf0,
};

namespace detail {

constexpr std::array<const char *, 256> makeResultCodeNameTable()
{
    std::array<const char *, 256> result{};

#ifdef REGISTER
#error
#endif
#define REGISTER(value) result[static_cast<size_t>(ResultCode::value)] = #value

    REGISTER(OK);
    REGISTER(OK_INITIALIZED);

    REGISTER(READ_OK);
    REGISTER(READ_BUFFER_FULL);
    REGISTER(READ_OBJECT_END);
    REGISTER(READ_END);

    REGISTER(WRITE_OK);
    REGISTER(WRITE_BUFFER_UNDERFULL);
    REGISTER(WRITE_OBJECT_END);
    REGISTER(WRITE_END);

    REGISTER(WARNING);
    REGISTER(WARNING_NOP);
    REGISTER(WARNING_INPUT_NOP);
    REGISTER(WARNING_DOUBLE_INIT);

    REGISTER(ERROR);

    REGISTER(USER_ERROR);

    REGISTER(USER_ERROR_MISSING_INFO);
    REGISTER(USER_ERROR_MISSING_PALETTE);
    REGISTER(USER_ERROR_MISSING_BOUNDARIES);
    REGISTER(USER_ERROR_INVALID_FORMAT);
    REGISTER(USER_ERROR_INVALID_COLOR_FORMAT);

    REGISTER(READ_ERROR);
    REGISTER(READ_ERROR_UNEXPECTED_EOF);
    REGISTER(READ_ERROR_IO_FAIL);
    REGISTER(READ_ERROR_PARSE_FAIL);
    REGISTER(READ_ERROR_TEXT_DATA_PARSE_FAIL);
    REGISTER(READ_ERROR_UNEXPECTED_MAGIC);
    REGISTER(READ_ERROR_UNEXPECTED_SYMBOL);
    REGISTER(READ_ERROR_UNKNOWN_VERSION);
    REGISTER(READ_ERROR_UNKNOWN_FEATURE);
    REGISTER(READ_ERROR_CORRUPTED_ENUM);
    REGISTER(READ_ERROR_CORRUPTED_BOOL);
    REGISTER(READ_ERROR_MISSING_DATA);
    REGISTER(READ_ERROR_WRONG_LIST_LENGTH);
    REGISTER(READ_ERROR_MULTIPLE_ROOTS);
    REGISTER(READ_ERROR_ILLEGAL_DATA_LENGTH);
    REGISTER(READ_ERROR_STRING_TOO_SHORT);
    REGISTER(READ_ERROR_INVALID_CONSTANT);
    REGISTER(READ_ERROR_INVALID_CHECKSUM);
    REGISTER(READ_ERROR_TEXT_DATA_PARSE_FAIL);

    REGISTER(READ_ERROR_SEMANTICAL);
    REGISTER(READ_ERROR_VALUE_OUT_OF_BOUNDS);
    REGISTER(READ_ERROR_UNSUPPORTED_VERSION);
    REGISTER(READ_ERROR_UNSUPPORTED_FEATURE);

    REGISTER(WRITE_ERROR);
    REGISTER(WRITE_ERROR_OUT_OF_BOUNDS);
    REGISTER(WRITE_ERROR_POSITION_OUT_OF_BOUNDS);
    REGISTER(WRITE_ERROR_INDEX_OUT_OF_BOUNDS);
    REGISTER(WRITE_ERROR_IO_FAIL);
    REGISTER(WRITE_ERROR_UNSUPPORTED_FORMAT);

    REGISTER(INTERNAL_ERROR);

#undef REGISTER
    return result;
}

constexpr std::array<const char *, 256> RESULT_CODE_NAME_TABLE = makeResultCodeNameTable();

}  // namespace detail

constexpr const char *nameOf(ResultCode code)
{
    return detail::RESULT_CODE_NAME_TABLE[static_cast<size_t>(code)];
}

constexpr bool isGood(ResultCode code)
{
    return code < ResultCode::WARNING;
}

constexpr bool isGoodOrWarning(ResultCode code)
{
    return code < ResultCode::ERROR;
}

constexpr bool isWarning(ResultCode code)
{
    return code >= ResultCode::WARNING && code < ResultCode::ERROR;
}

constexpr bool isErrorOrWarning(ResultCode code)
{
    return code >= ResultCode::WARNING;
}

constexpr bool isError(ResultCode code)
{
    return code >= ResultCode::ERROR;
}

constexpr bool isReadError(ResultCode code)
{
    return code >= ResultCode::READ_ERROR && code < ResultCode::WRITE_ERROR;
}

constexpr bool isWriteError(ResultCode code)
{
    return code >= ResultCode::WRITE_ERROR && code < ResultCode::INTERNAL_ERROR;
}

constexpr bool isInternalError(ResultCode code)
{
    return code >= ResultCode::INTERNAL_ERROR;
}

std::string informativeNameOf(ResultCode code);

static_assert(isGood(ResultCode::OK));
static_assert(isGood(ResultCode::READ_OK));
static_assert(isGood(ResultCode::WRITE_OK));

struct Error {
    uint64_t location;
    std::string what;
};

struct ReadResult {
    // GOOD RESULTS

    static ReadResult ok(uint64_t voxelsRead = 0)
    {
        return {voxelsRead, ResultCode::OK, std::nullopt};
    }

    static ReadResult incomplete(uint64_t voxelsRead = 0)
    {
        return {voxelsRead, ResultCode::READ_BUFFER_FULL, std::nullopt};
    }

    static ReadResult nextObject(uint64_t voxelsRead = 0)
    {
        return {voxelsRead, ResultCode::READ_OBJECT_END, std::nullopt};
    }

    static ReadResult end(uint64_t voxelsRead = 0)
    {
        return {voxelsRead, ResultCode::READ_END, std::nullopt};
    }

    // BAD RESULTS

    static ReadResult genericError(uint64_t location, std::string what)
    {
        return {0, ResultCode::ERROR, Error{location, std::move(what)}};
    }

    static ReadResult parseError(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_PARSE_FAIL, Error{location, std::move(what)}};
    }

    static ReadResult ioError(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_IO_FAIL, Error{location, std::move(what)}};
    }

    static ReadResult unexpectedEof(uint64_t location, std::string what = "Unexpected end of file (EOF)")
    {
        return {0, ResultCode::READ_ERROR_UNEXPECTED_EOF, Error{location, std::move(what)}};
    }

    static ReadResult unexpectedMagic(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNEXPECTED_MAGIC, Error{location, std::move(what)}};
    }

    static ReadResult unexpectedSymbol(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNEXPECTED_SYMBOL, Error{location, std::move(what)}};
    }

    static ReadResult unsupportedVersion(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNSUPPORTED_VERSION, Error{location, std::move(what)}};
    }

    static ReadResult unsupportedFeature(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNSUPPORTED_FEATURE, Error{location, std::move(what)}};
    }

    static ReadResult unknownVersion(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNKNOWN_VERSION, Error{location, std::move(what)}};
    }

    static ReadResult unknownFeature(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_UNKNOWN_FEATURE, Error{location, std::move(what)}};
    }

    static ReadResult missingHeaderField(uint64_t location, std::string what)
    {
        return {0, ResultCode::READ_ERROR_MISSING_DATA, Error{location, std::move(what)}};
    }

    uint64_t voxelsRead;
    ResultCode type;
    std::optional<Error> error;

    ReadResult() = default;

    ReadResult(uint64_t voxelsRead, ResultCode code, std::optional<Error> error = std::nullopt)
        : voxelsRead{voxelsRead}, type{code}, error{std::move(error)}
    {
    }

    ReadResult(const ReadResult &copyOf) = default;
    ReadResult(ReadResult &&moveOf) = default;

    ReadResult &operator=(const ReadResult &result) = default;
    ReadResult &operator=(ReadResult &&result) = default;

    constexpr operator ResultCode()
    {
        return type;
    }

    constexpr bool isGood() const
    {
        return ::voxelio::isGood(type);
    }

    constexpr bool isBad() const
    {
        return ::voxelio::isError(type);
    }

    constexpr bool isEnd() const
    {
        return type == ResultCode::READ_END;
    }
};

}  // namespace voxelio

#endif  // VXIO_RESULTCODE_HPP
