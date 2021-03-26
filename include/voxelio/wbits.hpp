#ifndef WBITS_HPP
#define WBITS_HPP

#include "bits.hpp"

namespace voxelio::wide {

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr usize popCount(const Int input[], usize count)
{
    usize result = 0;
    for (usize i = 0; i < count; ++i) {
        result += voxelio::popCount(input[i]);
    }
    return result;
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitClear(Int dest[], usize count)
{
    for (usize i = 0; i < count; ++i) {
        dest[i] = 0;
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitNot(Int out[], usize count)
{
    for (usize i = 0; i < count; ++i) {
        out[i] = ~out[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitAnd(Int dest[], const Int r[], usize count)
{
    for (usize i = 0; i < count; ++i) {
        dest[i] &= r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitOr(Int dest[], const Int r[], usize count)
{
    for (usize i = 0; i < count; ++i) {
        dest[i] |= r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr void bitXor(Int dest[], const Int r[], usize count)
{
    for (usize i = 0; i < count; ++i) {
        dest[i] ^= r[i];
    }
}

template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr void leftShift(Int dest[], Int shift, usize count)
{
    constexpr usize typeBits = sizeof(Int) * 8;

    VXIO_DEBUG_ASSERT_NE(count, 0u);

    for (; shift >= typeBits; shift -= typeBits) {
        for (usize i = count; i-- > 1;) {
            dest[i] = dest[i - 1];
        }
        dest[0] = 0;
    }

    if (shift == 0) {
        return;
    }

    Int carryMask = ~(~Int{0} << shift);

    dest[count - 1] <<= shift;

    for (usize i = count - 1; i-- > 0;) {
        Int shifted = voxelio::leftRot(dest[i], static_cast<unsigned char>(shift));
        dest[i + 0] = shifted & ~carryMask;
        dest[i + 1] = shifted & carryMask;
    }
}

template <typename Int, std::enable_if_t<std::is_unsigned_v<Int>, int> = 0>
constexpr void rightShift(Int dest[], Int shift, usize count)
{
    constexpr usize typeBits = sizeof(Int) * 8;

    VXIO_DEBUG_ASSERT_NE(count, 0u);

    for (; shift >= typeBits; shift -= typeBits) {
        for (usize i = 0; i < count - 1; ++i) {
            dest[i] = dest[i + 1];
        }
        dest[count - 1] = 0;
    }

    if (shift == 0) {
        return;
    }

    Int carryMask = ~(~Int{0} >> shift);

    dest[0] >>= shift;

    for (usize i = 1; i < count; ++i) {
        Int shifted = voxelio::rightRot(dest[i], static_cast<unsigned char>(shift));
        dest[i - 0] = shifted & ~carryMask;
        dest[i - 1] = shifted & carryMask;
    }
}

template <usize BITS, std::enable_if_t<BITS != 0, int> = 0>
class Bits {
public:
    using valueType = uintmax_t;

private:
    /** the size in value types of this BitVec */
    static constexpr usize size_ = divCeil(BITS, sizeof(valueType) * 8);
    /** the number of bits that spill into the topmost element (0 if none) */
    static constexpr usize bitSpill = BITS % (sizeof(valueType) * size_);
    /** the bit mask of the spillage into the topmost element or 0b111...111 if there is no spillage */
    static constexpr usize spillMask = ~valueType{0} << bitSpill;

    constexpr void fixBack()
    {
        data_[size() - 1] &= spillMask;
    }

    valueType data_[size_];

public:
    constexpr Bits() = default;
    constexpr Bits(const Bits &) = default;
    constexpr Bits(Bits &&) = default;

    constexpr Bits(valueType value) : data_{}
    {
        data_[0] = value;
        fixBack();
    }

    constexpr valueType *data()
    {
        return data_;
    }

    constexpr const valueType *data() const
    {
        return data_;
    }

    constexpr usize size() const
    {
        return size_;
    }

    void clear()
    {
        for (usize i = 0; i < size(); ++i) {
            data_[i] = 0;
        }
    }

    // UNARY OPERATORS =================================================================================================

    constexpr operator bool()
    {
        for (usize i = 0; i < size(); ++i) {
            if (data_[i]) return true;
        }
        return false;
    }

    constexpr Bits &operator~()
    {
        wide::bitNot(data(), size());
        fixBack();
        return *this;
    }

    // ASSIGNMENT OPERATORS ============================================================================================

    constexpr Bits &operator=(const Bits &) = default;
    constexpr Bits &operator=(Bits &&) = default;

    constexpr Bits &operator=(valueType value)
    {
        clear();
        data_[0] = value;
        return *this;
    }

    constexpr Bits &operator&=(Bits other)
    {
        wide::bitAnd(data(), other.data(), size());
        return *this;
    }

    constexpr Bits &operator|=(Bits other)
    {
        wide::bitOr(data(), other.data(), size());
        return *this;
    }

    constexpr Bits &operator^=(Bits other)
    {
        wide::bitXor(data(), other.data(), size());
        return *this;
    }

    constexpr Bits &operator<<=(valueType shift)
    {
        wide::leftShift(data(), shift, size());
        fixBack();
        return *this;
    }

    constexpr Bits &operator>>=(valueType shift)
    {
        wide::rightShift(data(), shift, size());
        return *this;
    }

    // BINARY OPERATORS ================================================================================================

    constexpr Bits operator&(Bits other)
    {
        wide::bitAnd(other.data(), data(), size());
        return other;
    }

    constexpr Bits operator|(Bits other)
    {
        wide::bitOr(other.data(), data(), size());
        return other;
    }

    constexpr Bits operator^(Bits other)
    {
        wide::bitXor(other.data(), data(), size());
        return other;
    }

    constexpr Bits operator<<(valueType shift)
    {
        Bits copy = *this;
        wide::leftShift(copy.data(), shift, size());
        copy.fixBack();
        return copy;
    }

    constexpr Bits operator>>(valueType shift)
    {
        Bits copy = *this;
        wide::rightShift(copy.data(), shift, size());
        return copy;
    }
};

template class Bits<128>;

}  // namespace voxelio::wide

#endif  // WBITS_HPP
