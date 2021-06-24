#ifndef VXIO_TEST_RANDOM_HPP
#define VXIO_TEST_RANDOM_HPP

#include "voxelio/primitives.hpp"

#include <random>

namespace voxelio::test {

using fast_rng32 = std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0>;
using fast_rng64 = std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, 0>;
using default_rng = std::mt19937;

constexpr std::uint32_t DEFAULT_SEED = 12345;

inline std::uint32_t hardwareSeed()
{
    return std::random_device{}();
}

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

void makeRandomData(u8 *out, usize length, u32 seed = 12345) noexcept;

void makeRandomIndexSequence(std::size_t *out, usize length, u32 seed) noexcept;

void makeRandomChars(char *out, size_t length, StringType type, u32 seed, u32 charRepititions) noexcept;

}  // namespace voxelio::test

#endif
