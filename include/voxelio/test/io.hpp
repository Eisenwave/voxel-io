#ifndef VXIO_TEST_IO_HPP
#define VXIO_TEST_IO_HPP

#include "voxelio/test/random.hpp"

#include "voxelio/primitives.hpp"

namespace voxelio::test {

enum class StringType {
    /** String of random bytes [0,0xff] */
    BYTE,
    /** String of any ASCII characters [0, 0x7f] */
    ASCII,
    /** String of ASCII printable characters. */
    PRINTABLE,
    /** String of lower case letters. */
    LOWER_CHARS,
    /** String of upper case letters. */
    UPPER_CHARS,
    /** String of binary digits. */
    BINARY,
    /** String of decimal digits. */
    DECIMAL
};

std::string makeRandomString(usize length,
                             StringType type = StringType::PRINTABLE,
                             u32 seed = 12345,
                             u32 charRepititions = 1);

void dumpString(const std::string &path, const std::string &str);

const std::string &getOutputPath();

inline std::string suggestRandomOutputPath()
{
    return getOutputPath() + makeRandomString(16, StringType::LOWER_CHARS, hardwareSeed());
}

}  // namespace voxelio::test

#endif  // IO_HPP
