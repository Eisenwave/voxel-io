#include "voxelio/test/random.hpp"
#include "voxelio/test/test.hpp"

#include "voxelio/bits.hpp"
#include "voxelio/endian.hpp"
#include "voxelio/ileave.hpp"
#include "voxelio/vec.hpp"
#include "voxelio/wbits.hpp"
#include "voxelio/wileave.hpp"

#include <ostream>  // for vec stringification
#include <random>

namespace voxelio::test {
namespace {

// STATIC ==============================================================================================================

template <uint32_t (*log2_floor_32)(uint32_t), uint64_t (*log2_floor_64)(uint64_t) = nullptr>
void test_log2_floor_forPow2()
{
    for (size_t i = 1; i < 32; ++i) {
        VXIO_ASSERT_EQ(log2_floor_32(uint32_t{1} << i), i);
    }
    if (log2_floor_64 == nullptr) {
        return;
    }
    for (size_t i = 1; i < 64; ++i) {
        VXIO_ASSERT_EQ(log2_floor_64(uint64_t{1} << i), i);
    }
}

template <uint32_t (*log2_floor_32)(uint32_t), uint64_t (*log2_floor_64)(uint64_t) = nullptr>
void test_log2_floor_general()
{
    for (size_t i = 4; i < 32; ++i) {
        uint32_t input = uint32_t{1} << i;
        VXIO_ASSERT_EQ(log2_floor_32(input), i);
        VXIO_ASSERT_EQ(log2_floor_32(input + 1), i);
        VXIO_ASSERT_EQ(log2_floor_32(input + 2), i);
        VXIO_ASSERT_EQ(log2_floor_32(input + 3), i);
    }
    if (log2_floor_64 == nullptr) {
        return;
    }
    for (size_t i = 4; i < 64; ++i) {
        uint64_t input = uint64_t{1} << i;
        VXIO_ASSERT_EQ(log2_floor_64(input), i);
        VXIO_ASSERT_EQ(log2_floor_64(input + 1), i);
        VXIO_ASSERT_EQ(log2_floor_64(input + 2), i);
        VXIO_ASSERT_EQ(log2_floor_64(input + 3), i);
    }
}

// TESTS ===============================================================================================================

VXIO_TEST(intlog, log2floor_naive_forPow2)
{
    test_log2_floor_forPow2<log2floor_naive<uint32_t>, log2floor_naive<uint64_t>>();
}

VXIO_TEST(intlog, log2floor_fast_forPow2)
{
    test_log2_floor_forPow2<log2floor_fast<uint32_t>, log2floor_fast<uint64_t>>();
}

VXIO_TEST(intlog, log2floor_debruijn_forPow2)
{
    test_log2_floor_forPow2<log2floor_debruijn>();
}

VXIO_TEST(intlog, log2floor_forPow2)
{
    test_log2_floor_forPow2<log2floor<uint32_t>, log2floor<uint64_t>>();
}

VXIO_TEST(intlog, log2floor_naive_general)
{
    test_log2_floor_general<log2floor_naive<uint32_t>, log2floor_naive<uint64_t>>();
}

VXIO_TEST(intlog, log2floor_fast_general)
{
    test_log2_floor_general<log2floor_fast<uint32_t>, log2floor_fast<uint64_t>>();
}

VXIO_TEST(intlog, log2floor_debruijn_general)
{
    test_log2_floor_general<log2floor_debruijn>();
}

VXIO_TEST(intlog, log2floor_general)
{
    test_log2_floor_general<log2floor_debruijn>();
}

VXIO_TEST(intlog, log2floor_manual)
{
    VXIO_STATIC_ASSERT_EQ(log2floor(0u), 0u);
    VXIO_STATIC_ASSERT_EQ(log2floor_debruijn(0u), 0u);
    VXIO_STATIC_ASSERT_EQ(log2floor_fast(0u), 0u);
    VXIO_STATIC_ASSERT_EQ(log2floor_naive(0u), 0u);
}

VXIO_TEST(intlog, log10floor_manual)
{
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(0u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(9u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(10u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(99u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(100u), 2u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint8_t>(255u), 2u);

    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(0u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(1u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(2u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(3u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(4u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(5u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(6u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(7u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(8u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(9u), 0u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(10u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(11u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(12u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(13u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(14u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(15u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(16u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(99u), 1u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(100u), 2u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(999u), 2u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(1'000u), 3u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(9'999u), 3u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(10000u), 4u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(99'999u), 4u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(100'000u), 5u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(999'999u), 5u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(1'000'000u), 6u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(9'999'999u), 6u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(10'000'000u), 7u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(99'999'999u), 7u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(100'000'000u), 8u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(999'999'999u), 8u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(1'000'000'000u), 9u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(2'000'000'000u), 9u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(4'000'000'000u), 9u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint32_t>(uint32_t(~0)), 9u);

    VXIO_STATIC_ASSERT_EQ(log10floor<uint64_t>(uint64_t{1} << 63), 18u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint64_t>(9'999'999'999'999'999'999ull), 18u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint64_t>(10'000'000'000'000'000'000ull), 19u);
    VXIO_STATIC_ASSERT_EQ(log10floor<uint64_t>(~uint64_t{0}), 19u);
}

VXIO_TEST(ileave, ileaveZeros_naive_manual)
{
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xff, 0), 0xffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xff, 1), 0b0101'0101'0101'0101u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xff, 2), 0b001001001001001001001001u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 1), 0x5555'5555'5555'5555u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 2), 0x9249'2492'4924'9249u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 3), 0x1111'1111'1111'1111u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 7), 0x0101'0101'0101'0101u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 15), 0x0001'0001'0001'0001u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::ileaveZeros_naive(0xffff'ffff, 31), 0x0000'0001'0000'0001u);

    VXIO_STATIC_ASSERT_EQ(ileaveZeros_const<4>(12345678), voxelio::detail::ileaveZeros_naive(12345678, 4));
}

VXIO_TEST(ileave, duplBits_naive_manual)
{
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(0xf, 0), 0u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(0xf, 2), 0xffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(0x55, 2), 0x3333u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(0xff, 2), 0xffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 1), 1u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 2), 3u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 4), 0xfu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 8), 0xffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 16), 0xffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 32), 0xffffffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(1, 64), 0xffffffffffffffffu);
}

VXIO_TEST(ileave, ileaveBits_and_duplBits_manual)
{
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 1), 1),
                          0x5555'5555'5555'5555u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 1), 2),
                          0x3333'3333'3333'3333u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 1), 4),
                          0x0f0f'0f0f'0f0f'0f0fu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 1), 8),
                          0x00ff'00ff'00ff'00ffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 1), 16),
                          0x0000'ffff'0000'ffffu);

    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 2), 1),
                          0x9249'2492'4924'9249u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 2), 2),
                          0x30C3'0C30'C30C'30C3u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 2), 4),
                          0xF00F'00F0'0F00'F00Fu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 2), 8),
                          0x00FF'0000'FF00'00FFu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 2), 16),
                          0xFFFF'0000'0000'FFFFu);

    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 3), 1),
                          0x1111'1111'1111'1111u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 3), 2),
                          0x0303'0303'0303'0303u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 3), 4),
                          0x000f'000f'000f'000fu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 3), 8),
                          0x0000'00ff'0000'00ffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::duplBits_naive(voxelio::detail::ileaveZeros_naive(uint32_t(-1), 3), 16),
                          0xffffu);
}

VXIO_TEST(ileave, remIleavedBits_naive_manual)
{
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0xff, 0), 0xffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0xff, 1), 0xfu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0b01010101, 1), 0b1111u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x5555'5555'5555'5555, 1), 0xffff'ffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x1111'1111'1111'1111, 3), 0b1111'1111'1111'1111u);

    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x5555'5555'5555'5555, 0), 0x5555'5555'5555'5555u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x5, 1), 3u);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x5555'5555'5555'5555, 1), 0xffff'ffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0xffff'ffff'ffff'ffff, 1), 0xffff'ffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0xffff'ffff'ffff'ffff, 1), 0xffff'ffffu);
    VXIO_STATIC_ASSERT_EQ(voxelio::detail::remIleavedBits_naive(0x9249'2492'4924'9249, 2), 0x3fffffu);
}

VXIO_TEST(ileave, remIleavedBits_const_manual)
{
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<0>(0xff), 0xffu);
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0xff), 0xfu);
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0b01010101), 0b1111u);
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0x5555'5555'5555'5555), 0xffff'ffffu);
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<3>(0x1111'1111'1111'1111), 0b1111'1111'1111'1111u);

    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<4>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 4));
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<8>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 8));
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<16>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 16));
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<32>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 32));
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<63>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 63));
    VXIO_STATIC_ASSERT_EQ(remIleavedBits_const<64>(12345678), voxelio::detail::remIleavedBits_naive(12345678, 64));
}

VXIO_TEST(ileave, remIleavedBits_naive_matches_templated)
{
    constexpr size_t iterations = 1024 * 8;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<uint64_t> distr{0, std::numeric_limits<uint64_t>::max()};

    for (size_t i = 0; i < iterations; ++i) {
        const auto input = distr(rng);
        VXIO_ASSERT_EQ(remIleavedBits_const<1>(input), voxelio::detail::remIleavedBits_naive(input, 1));
        VXIO_ASSERT_EQ(remIleavedBits_const<2>(input), voxelio::detail::remIleavedBits_naive(input, 2));
        VXIO_ASSERT_EQ(remIleavedBits_const<3>(input), voxelio::detail::remIleavedBits_naive(input, 3));
        VXIO_ASSERT_EQ(remIleavedBits_const<4>(input), voxelio::detail::remIleavedBits_naive(input, 4));
        VXIO_ASSERT_EQ(remIleavedBits_const<5>(input), voxelio::detail::remIleavedBits_naive(input, 5));
        VXIO_ASSERT_EQ(remIleavedBits_const<6>(input), voxelio::detail::remIleavedBits_naive(input, 6));
        VXIO_ASSERT_EQ(remIleavedBits_const<7>(input), voxelio::detail::remIleavedBits_naive(input, 7));
        VXIO_ASSERT_EQ(remIleavedBits_const<8>(input), voxelio::detail::remIleavedBits_naive(input, 8));
        VXIO_ASSERT_EQ(remIleavedBits_const<9>(input), voxelio::detail::remIleavedBits_naive(input, 9));
        VXIO_ASSERT_EQ(remIleavedBits_const<10>(input), voxelio::detail::remIleavedBits_naive(input, 10));
        VXIO_ASSERT_EQ(remIleavedBits_const<11>(input), voxelio::detail::remIleavedBits_naive(input, 11));
        VXIO_ASSERT_EQ(remIleavedBits_const<12>(input), voxelio::detail::remIleavedBits_naive(input, 12));
        VXIO_ASSERT_EQ(remIleavedBits_const<13>(input), voxelio::detail::remIleavedBits_naive(input, 13));
        VXIO_ASSERT_EQ(remIleavedBits_const<14>(input), voxelio::detail::remIleavedBits_naive(input, 14));
        VXIO_ASSERT_EQ(remIleavedBits_const<15>(input), voxelio::detail::remIleavedBits_naive(input, 15));
        VXIO_ASSERT_EQ(remIleavedBits_const<16>(input), voxelio::detail::remIleavedBits_naive(input, 16));
        VXIO_ASSERT_EQ(remIleavedBits_const<32>(input), voxelio::detail::remIleavedBits_naive(input, 32));
        VXIO_ASSERT_EQ(remIleavedBits_const<63>(input), voxelio::detail::remIleavedBits_naive(input, 63));
    }
}

VXIO_TEST(ileave, ileaveZeros_naive_matches_template)
{
    constexpr size_t iterations = 1024 * 8;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<uint32_t> distr{0, std::numeric_limits<uint32_t>::max()};

    for (size_t i = 0; i < iterations; ++i) {
        const auto input = distr(rng);
        VXIO_ASSERT_EQ(ileaveZeros_const<1>(input), voxelio::detail::ileaveZeros_naive(input, 1));
        VXIO_ASSERT_EQ(ileaveZeros_const<2>(input), voxelio::detail::ileaveZeros_naive(input, 2));
        VXIO_ASSERT_EQ(ileaveZeros_const<3>(input), voxelio::detail::ileaveZeros_naive(input, 3));
        VXIO_ASSERT_EQ(ileaveZeros_const<4>(input), voxelio::detail::ileaveZeros_naive(input, 4));
        VXIO_ASSERT_EQ(ileaveZeros_const<5>(input), voxelio::detail::ileaveZeros_naive(input, 5));
        VXIO_ASSERT_EQ(ileaveZeros_const<6>(input), voxelio::detail::ileaveZeros_naive(input, 6));
        VXIO_ASSERT_EQ(ileaveZeros_const<7>(input), voxelio::detail::ileaveZeros_naive(input, 7));
        VXIO_ASSERT_EQ(ileaveZeros_const<8>(input), voxelio::detail::ileaveZeros_naive(input, 8));
        VXIO_ASSERT_EQ(ileaveZeros_const<9>(input), voxelio::detail::ileaveZeros_naive(input, 9));
        VXIO_ASSERT_EQ(ileaveZeros_const<10>(input), voxelio::detail::ileaveZeros_naive(input, 10));
        VXIO_ASSERT_EQ(ileaveZeros_const<11>(input), voxelio::detail::ileaveZeros_naive(input, 11));
        VXIO_ASSERT_EQ(ileaveZeros_const<12>(input), voxelio::detail::ileaveZeros_naive(input, 12));
        VXIO_ASSERT_EQ(ileaveZeros_const<13>(input), voxelio::detail::ileaveZeros_naive(input, 13));
        VXIO_ASSERT_EQ(ileaveZeros_const<14>(input), voxelio::detail::ileaveZeros_naive(input, 14));
        VXIO_ASSERT_EQ(ileaveZeros_const<15>(input), voxelio::detail::ileaveZeros_naive(input, 15));
        VXIO_ASSERT_EQ(ileaveZeros_const<16>(input), voxelio::detail::ileaveZeros_naive(input, 16));
        VXIO_ASSERT_EQ(ileaveZeros_const<16>(input), voxelio::detail::ileaveZeros_naive(input, 16));
        VXIO_ASSERT_EQ(ileaveZeros_const<32>(input), voxelio::detail::ileaveZeros_naive(input, 32));
        VXIO_ASSERT_EQ(ileaveZeros_const<63>(input), voxelio::detail::ileaveZeros_naive(input, 63));
    }
}

VXIO_TEST(ileave, ileave_manual)
{
    VXIO_STATIC_ASSERT_EQ(ileave2(0b1111'1111, 0), 0b1010'1010'1010'1010u);
    VXIO_STATIC_ASSERT_EQ(ileave2(0, 0b1'1111'1111), 0b01'0101'0101'0101'0101u);
    VXIO_STATIC_ASSERT_EQ(ileave2(0, 0xffff'ffff), 0x5555'5555'5555'5555u);
    VXIO_STATIC_ASSERT_EQ(ileave2(0, ileave2(0, 0b11)), 0b10001u);

    VXIO_STATIC_ASSERT_EQ(ileave3(0, 0, 0b1111), 0b001001001001u);
    VXIO_STATIC_ASSERT_EQ(ileave3(0b1111, 0, 0), 0b100100100100u);
}

VXIO_TEST(ileave, ileaveBytes_manual)
{
    for (size_t i = 0; i <= 8; ++i) {
        VXIO_ASSERT_EQ(ileaveBytes(0, i), 0u);
    }

    VXIO_ASSERT_EQ(ileaveBytes(0xcc, 1), 0xccu);

    VXIO_ASSERT_EQ(ileaveBytes(0xff, 2), 0x5555u);
    VXIO_ASSERT_EQ(ileaveBytes(0xff00, 2), 0xaaaau);

    VXIO_ASSERT_EQ(ileaveBytes(0x0000ff, 3), 0b001'001'001'001'001'001'001'001u << 0);
    VXIO_ASSERT_EQ(ileaveBytes(0x00ff00, 3), 0b001'001'001'001'001'001'001'001u << 1);
    VXIO_ASSERT_EQ(ileaveBytes(0xff0000, 3), 0b001'001'001'001'001'001'001'001u << 2);

    VXIO_ASSERT_EQ(ileaveBytes(0x000000ff, 8), 0x0101'0101'0101'0101u);
    VXIO_ASSERT_EQ(ileaveBytes(0x0000ff00, 8), 0x0202'0202'0202'0202u);
    VXIO_ASSERT_EQ(ileaveBytes(0x00ff0000, 8), 0x0404'0404'0404'0404u);
    VXIO_ASSERT_EQ(ileaveBytes(0xff000000, 8), 0x0808'0808'0808'0808u);

    VXIO_ASSERT_EQ(ileaveBytes(0xff000000ff, 8), 0x1111'1111'1111'1111u);
}

VXIO_TEST(ileave, ileaveBytes_naive_matches_jmp)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        uint64_t bytesAsInt = distr(rng);

        uint64_t jmp = voxelio::detail::ileaveBytes_jmp(bytesAsInt, i % 9);
        uint64_t naive = voxelio::detail::ileaveBytes_naive(bytesAsInt, i % 9);
        VXIO_ASSERT_EQ(jmp, naive);
    }
}

VXIO_TEST(ileave, ileaveBytes_bitCountPreserved)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        uint8_t count = i % 9;
        uint64_t next = distr(rng);
        uint64_t bytesAsInt = count == 0 ? 0 : next >> ((8 - count) * 8);

        uint64_t ileaved = ileaveBytes(bytesAsInt, count);
        VXIO_ASSERT_EQ(popCount(ileaved), popCount(bytesAsInt));
    }
}

VXIO_TEST(ileave, ileaveBytes_dileaveBytes_random)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        uint8_t count = i % 9;
        uint64_t next = distr(rng);
        uint64_t bytesAsInt = count == 0 ? 0 : next >> ((8 - count) * 8);

        uint64_t ileaved = ileaveBytes(bytesAsInt, count);
        uint64_t dileaved = dileave_bytes(ileaved, count);

        VXIO_ASSERT_EQ(dileaved, bytesAsInt);
    }
}

VXIO_TEST(ileave, ileave3_naive_matches_regular)
{
    std::mt19937 rng{12345};
    std::uniform_int_distribution<uint32_t> distr{0, std::numeric_limits<uint32_t>::max()};

    for (size_t i = 0; i < 1024 * 1; ++i) {
        const Vec3u32 v = {distr(rng), distr(rng), distr(rng)};
        VXIO_ASSERT_EQ(ileave3(v.x(), v.y(), v.z()), voxelio::detail::ileave3_naive(v.x(), v.y(), v.z()));
    }
}

VXIO_TEST(ileave, dileave3_reverses_ileave3)
{
    std::mt19937 rng{12345};
    std::uniform_int_distribution<uint32_t> distr{0, 1u << 21};

    for (size_t i = 0; i < 1024 * 1; ++i) {
        const auto x = distr(rng), y = distr(rng), z = distr(rng);
        Vec3u32 expected = {x, y, z};
        Vec3u32 actual;
        voxelio::detail::dileave3_naive(ileave3(x, y, z), actual.data());
        VXIO_ASSERT_EQ(actual, expected);
    }
}

template <typename T>
void clear(T data[8])
{
    for (size_t i = 0; i < 8; ++i) {
        data[i] = T{};
    }
}

template <typename UInt,
          void (*WILEAVE)(const UInt *, uint64_t *, size_t),
          void (*WDILEAVE)(const uint64_t *, UInt *, size_t)>
static void test_wileave_inverse()
{
    constexpr size_t iterations = 1024;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<uint64_t> distr;
    UInt input[8]{};
    uint64_t output[8]{};
    UInt inverse[8]{};

    for (size_t i = 0; i < iterations; ++i) {
        // size_t count = 1;
        size_t count = distr(rng) % 9;
        for (size_t j = 0; j < count; ++j) {
            input[j] = static_cast<UInt>(distr(rng));
        }

        WILEAVE(input, output, count);
        WDILEAVE(output, inverse, count);

        size_t inputBitCount = wide::popCount(input, count);
        size_t outputBitCount = wide::popCount(output, count);
        size_t inverseBitCount = wide::popCount(inverse, count);

        VXIO_ASSERT_EQ(inputBitCount, outputBitCount);
        VXIO_ASSERT_EQ(inputBitCount, inverseBitCount);

        for (size_t j = 0; j < 8; ++j) {
            VXIO_ASSERT_EQ(input[j], inverse[j]);
        }

        clear(input);
        clear(output);
        clear(inverse);
    }
}

template <typename UInt,
          void (*WILEAVE0)(const UInt *, uint64_t *, size_t),
          void (*WILEAVE1)(const UInt *, uint64_t *, size_t)>
[[maybe_unused]] void test_wileave_equivalency()
{
    constexpr size_t iterations = 1024;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<uint64_t> distr;
    UInt input[8]{};
    uint64_t output[8]{};
    UInt inverse[8]{};

    for (size_t i = 0; i < iterations; ++i) {
        // size_t count = 1;
        size_t count = distr(rng) & 0b111;
        for (size_t j = 0; j < count; ++j) {
            input[j] = static_cast<UInt>(distr(rng));
        }

        WILEAVE(input, output, count);
        wide::detail::dileave_naive(output, inverse, count);

        size_t inputBitCount = wide::popCount(input, count);
        size_t outputBitCount = wide::popCount(output, count);
        size_t inverseBitCount = wide::popCount(inverse, count);

        VXIO_ASSERT_EQ(inputBitCount, outputBitCount);
        VXIO_ASSERT_EQ(inputBitCount, inverseBitCount);

        for (size_t j = 0; j < 8; ++j) {
            VXIO_ASSERT_EQ(input[j], inverse[j]);
        }

        clear(input);
        clear(output);
        clear(inverse);
    }
}

VXIO_TEST(wileave, naive_wdileave_naive_inverse_8)
{
    test_wileave_inverse<uint8_t, wide::detail::ileave_naive, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, naive_wdileave_naive_inverse_16)
{
    test_wileave_inverse<uint16_t, wide::detail::ileave_naive, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, naive_wdileave_naive_inverse_32)
{
    test_wileave_inverse<uint32_t, wide::detail::ileave_naive, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, naive_wdileave_naive_inverse_64)
{
    test_wileave_inverse<uint64_t, wide::detail::ileave_naive, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, wdileave_naive_inverse_8)
{
    test_wileave_inverse<uint8_t, wide::ileave, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, wdileave_naive_inverse_16)
{
    test_wileave_inverse<uint16_t, wide::ileave, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, wdileave_naive_inverse_32)
{
    test_wileave_inverse<uint32_t, wide::ileave, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, wdileave_naive_inverse_64)
{
    test_wileave_inverse<uint64_t, wide::ileave, wide::detail::dileave_naive>();
}

VXIO_TEST(wileave, wdileave_inverse_8)
{
    test_wileave_inverse<uint8_t, wide::ileave, wide::dileave>();
}

VXIO_TEST(wileave, wdileave_inverse_16)
{
    test_wileave_inverse<uint16_t, wide::ileave, wide::dileave>();
}

VXIO_TEST(wileave, wdileave_inverse_32)
{
    test_wileave_inverse<uint32_t, wide::ileave, wide::dileave>();
}

VXIO_TEST(wileave, wdileave_inverse_64)
{
    test_wileave_inverse<uint64_t, wide::ileave, wide::dileave>();
}

}  // namespace
}  // namespace voxelio::test
