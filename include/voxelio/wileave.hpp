#ifndef VXIO_WILEAVE_HPP
#define VXIO_WILEAVE_HPP
/*
 * wileave.hpp
 * -----------
 * This header provides wide bit interleaving and de-interleaving functions.
 */

#include "ileave.hpp"

namespace voxelio::wide {

// WIDE INTERLEAVING ===================================================================================================

template <usize COUNT, typename Uint, std::enable_if_t<COUNT <= 8 && std::is_unsigned_v<Uint>, int> = 0>
constexpr void ileave_const([[maybe_unused]] const Uint inputs[], [[maybe_unused]] uint64_t outputs[])
{
    constexpr usize inputBytes = COUNT * sizeof(Uint);
    constexpr usize outputSize = voxelio::divCeil(inputBytes, sizeof(uint64_t));
    // constexpr usize outputBytes = outputSize * sizeof (uint64_t);

    if constexpr (COUNT == 0) {
        return;
    }
    else if constexpr (COUNT == 1) {
        outputs[0] = inputs[0];
    }
    else {
        static_assert(outputSize != 0);

        for (usize o = 0; o < outputSize; ++o) {
            outputs[o] = 0;
        }

        for (usize i = 0; i < COUNT; ++i) {
            Uint input = inputs[i];
            if constexpr (outputSize == 1) {
                uint64_t result = ileaveZeros_const<COUNT - 1>(input) << i;
                outputs[0] |= result;
            }
            else if constexpr (voxelio::isPow2or0(COUNT)) {
                constexpr usize rshift = sizeof(Uint) * 8 / outputSize;

                for (usize j = 0; j < outputSize; ++j) {
                    uint64_t result = ileaveZeros_const<COUNT - 1>(static_cast<uint32_t>(input)) << i;
                    outputs[j] |= result;
                    input >>= rshift;
                }
            }
            else {
                // writeIndex is the index of current byte to write
                for (usize writeIndex = 0, nextIndex = 0; writeIndex < inputBytes; writeIndex = nextIndex) {
                    // clang-format off
                    nextIndex += COUNT;
                    uint8_t nextByte = input & 0xff;                             // mask single byte
                    uint64_t result = ileaveZeros_const<COUNT - 1>(nextByte) << i; // perform zero-interleaving

                    usize outputIndex = writeIndex / sizeof(uint64_t);       // get index in output
                    usize outputShift = writeIndex % sizeof(uint64_t) * 8;

                    outputs[outputIndex] |= result << outputShift;            // write result to output

                    usize nextOutputIndex = nextIndex / sizeof(uint64_t);    // get next index in output
                    if (nextOutputIndex != outputIndex) {                     // detect spill
                        usize spillIndex = nextIndex % sizeof(uint64_t);
                        usize spillShift = (COUNT - spillIndex) * 8;
                        outputs[nextOutputIndex] |= result >> spillShift;
                    }

                    input >>= 8;                                              // rightshift next byte into place
                    // clang-format on
                }
            }
        }
    }
}

namespace detail {

template <typename Uint>
constexpr void ileave_naive(const Uint inputs[], uint64_t outputs[], usize count)
{
    constexpr Uint singleInputBits = sizeof(Uint) * 8;
    constexpr Uint singleOutputBits = sizeof(uint64_t) * 8;

    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    if (count == 0) {
        return;
    }
    else if (count == 1) {
        outputs[0] = inputs[0];
        return;
    }
    const usize outputSize = voxelio::divCeil(count * sizeof(Uint), sizeof(uint64_t));
    for (usize o = 0; o < outputSize; ++o) {
        outputs[o] = 0;
    }

    const usize bits = count * singleInputBits;

    usize inputShift = 0;
    usize outputIndex = 0;

    for (usize b = 0, i = 0, o = 0; b < bits; ++b, ++i, ++o) {
        if (i == count) {
            i = 0;
            ++inputShift;
        }
        if (o == singleOutputBits) {
            o = 0;
            ++outputIndex;
        }
        bool bit = (inputs[i] >> inputShift) & 1;
        outputs[outputIndex] |= uint64_t{bit} << o;
    }
}

// alternative implementation adapting ileave_bytes_const to work with a runtime parameter
template <typename Uint>
constexpr void ileave_jmp(const Uint inputs[], uint64_t outputs[], usize count)
{
    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    switch (count) {
    case 0: wide::ileave_const<0>(inputs, outputs); return;
    case 1: wide::ileave_const<1>(inputs, outputs); return;
    case 2: wide::ileave_const<2>(inputs, outputs); return;
    case 3: wide::ileave_const<3>(inputs, outputs); return;
    case 4: wide::ileave_const<4>(inputs, outputs); return;
    case 5: wide::ileave_const<5>(inputs, outputs); return;
    case 6: wide::ileave_const<6>(inputs, outputs); return;
    case 7: wide::ileave_const<7>(inputs, outputs); return;
    case 8: wide::ileave_const<8>(inputs, outputs); return;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

}  // namespace detail

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr void ileave(const Uint inputs[], uint64_t outputs[], usize count)
{
    wide::detail::ileave_jmp<Uint>(inputs, outputs, count);
}

// WIDE DEINTERLEAVING =================================================================================================

namespace detail {
template <typename Uint>
constexpr void dileave_naive(const uint64_t inputs[], Uint outputs[], usize count);
}

template <usize COUNT, typename Uint, std::enable_if_t<COUNT <= 8 && std::is_unsigned_v<Uint>, int> = 0>
constexpr void dileave_const([[maybe_unused]] const uint64_t inputs[], [[maybe_unused]] Uint outputs[])
{
    constexpr usize outputBytes = COUNT * sizeof(Uint);
    constexpr usize inputSize = voxelio::divCeil(outputBytes, sizeof(uint64_t));
    // constexpr usize outputBytes = outputSize * sizeof (uint64_t);

    if constexpr (COUNT == 0) {
        return;
    }
    else if constexpr (COUNT == 1) {
        outputs[0] = static_cast<Uint>(inputs[0]);
    }
    else {
        static_assert(inputSize != 0);

        if constexpr (inputSize == 1) {
            for (usize o = 0; o < COUNT; ++o) {
                uint64_t dileaved = remIleavedBits_const<COUNT - 1>(inputs[0] >> o);
                outputs[o] = static_cast<Uint>(dileaved);
            }
        }
        else if constexpr (voxelio::isPow2or0(COUNT)) {
            for (usize o = 0; o < COUNT; ++o) {
                Uint result = 0;

                constexpr usize lshift = sizeof(Uint) * 8 / inputSize;

                for (usize j = inputSize; j != 0; --j) {
                    result <<= lshift;
                    result |= remIleavedBits_const<COUNT - 1>(inputs[j - 1] >> o);
                }

                outputs[o] = result;
            }
        }
        else {
            return wide::detail::dileave_naive(inputs, outputs, COUNT);
        }
    }
}

namespace detail {

template <typename Uint>
constexpr void dileave_naive(const uint64_t inputs[], Uint outputs[], usize count)
{
    constexpr Uint singleInputBits = sizeof(uint64_t) * 8;
    constexpr Uint singleOutputBits = sizeof(Uint) * 8;

    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    if (count == 0) {
        return;
    }
    else if (count == 1) {
        outputs[0] = static_cast<Uint>(inputs[0]);
        return;
    }
    for (usize o = 0; o < count; ++o) {
        outputs[o] = 0;
    }

    const usize bits = count * singleOutputBits;

    usize inputIndex = 0;
    usize outputShift = 0;

    for (usize b = 0, i = 0, o = 0; b < bits; ++b, ++i, ++o) {
        if (i == singleInputBits) {
            i = 0;
            ++inputIndex;
        }
        if (o == count) {
            o = 0;
            ++outputShift;
        }
        bool bit = (inputs[inputIndex] >> i) & 1;
        outputs[o] |= Uint{bit} << outputShift;
    }
}

template <typename Uint>
constexpr void dileave_jmp(const uint64_t inputs[], Uint outputs[], usize count)
{
    VXIO_DEBUG_ASSERT_LE(count, 8);
    VXIO_ASSUME(count <= 8);

    switch (count) {
    case 0: wide::dileave_const<0>(inputs, outputs); return;
    case 1: wide::dileave_const<1>(inputs, outputs); return;
    case 2: wide::dileave_const<2>(inputs, outputs); return;
    case 3: wide::dileave_const<3>(inputs, outputs); return;
    case 4: wide::dileave_const<4>(inputs, outputs); return;
    case 5: wide::dileave_const<5>(inputs, outputs); return;
    case 6: wide::dileave_const<6>(inputs, outputs); return;
    case 7: wide::dileave_const<7>(inputs, outputs); return;
    case 8: wide::dileave_const<8>(inputs, outputs); return;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

}  // namespace detail

template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr void dileave(const uint64_t inputs[], Uint outputs[], usize count)
{
    wide::detail::dileave_jmp<Uint>(inputs, outputs, count);
}

}  // namespace voxelio::wide

#endif  // WILEAVE_HPP
