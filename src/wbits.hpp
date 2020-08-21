#ifndef WBITS_HPP
#define WBITS_HPP

#include "bits.hpp"

namespace voxelio::wide {

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr size_t popCount(const Int input[], size_t count)
{
    size_t result = 0;
    for (size_t i = 0; i < count; ++i) {
        result += voxelio::popCount(input[i]);
    }
    return result;
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitClear(Int out[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        out[i] = 0;
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitNot(Int out[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        out[i] = ~out[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitAnd(const Int l[], const Int r[], Int out[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        out[i] = l[i] & r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitOr(const Int l[], const Int r[], Int out[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        out[i] = l[i] | r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitXor(const Int l[], const Int r[], Int out[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        out[i] = l[i] ^ r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void shiftl(Int in[], size_t count, size_t shift)
{
    constexpr size_t typeBits = sizeof(Int) * 8;

    VXIO_DEBUG_ASSERT_NE(count, 0);

    for (; shift >= typeBits; shift -= typeBits) {
        for (size_t i = count; i-- > 1;) {
            in[i] = in[i - 1];
        }
        in[0] = 0;
    }

    Int carryMask = (1 << shift) - 1;

    in[count - 1] <<= shift;

    for (size_t i = count - 1; i-- > 0;) {
        Int shifted = voxelio::rotl(in[i], static_cast<unsigned char>(shift));
        in[i + 0] = shifted & ~carryMask;
        in[i + 1] = shifted & carryMask;
    }
}

}  // namespace voxelio::wide

#endif  // WBITS_HPP
