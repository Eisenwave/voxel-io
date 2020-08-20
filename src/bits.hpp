#ifndef VXIO_BITS_HPP
#define VXIO_BITS_HPP

#include "assert.hpp"

#include "intdiv.hpp"
#include "intlog.hpp"

#include <cstddef>
#include <cstdint>

#ifndef VXIO_HAS_BUILTIN_POPCOUNT
#include <bitset>
#endif

namespace voxelio {

// BIT COUNTING ========================================================================================================

namespace detail {

// bitset operations are not constexpr so we need a naive implementation for constexpr context
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr unsigned char popCount_naive(Uint input)
{
    unsigned char result = 0;
    for (; input != 0; input >>= 1) {
        result += input & 1;
    }
    return result;
}

}  // namespace detail

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr unsigned char popCount(Int input)
{
    using Uint = std::make_unsigned_t<Int>;
#ifdef VXIO_HAS_BUILTIN_POPCOUNT
    int bits = voxelio::builtin::popCount(static_cast<Uint>(input));
    return static_cast<unsigned char>(bits);
#else
    if (voxelio::isConstantEvaluated()) {
        return detail::pop_count_naive(input);
    }
    else {
        std::bitset<std::numeric_limits<Uint>::digits> bits = static_cast<Uint>(input);
        return static_cast<unsigned char>(bits.count());
    }
#endif
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr size_t popCount(const Int input[], size_t count)
{
    size_t result = 0;
    for (size_t i = 0; i < count; ++i) {
        result += popCount(input[i]);
    }
    return result;
}

}  // namespace voxelio

#endif  // UTIL_BITS_HPP
