#include "voxelio/test/test.hpp"

#include "voxelio/parse.hpp"
#include "voxelio/stringify.hpp"
#include "voxelio/stringmanip.hpp"

#include <ostream>

namespace voxelio::test {
namespace {

using schar = signed char;

template <typename T>
void testParsing(schar expected)
{
#ifdef VXIO_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif
#ifdef VXIO_GNU
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

    T result = 0;
    VXIO_ASSERT(parse(std::to_string(expected), result));
    VXIO_ASSERT_EQ(result, static_cast<T>(expected));

#ifdef VXIO_CLANG
#pragma clang diagnostic pop
#endif
#ifdef VXIO_GNU
#pragma GCC diagnostic pop
#endif
}

template <typename T>
T parseUnsafe(const std::string &str)
{
    T result;
    VXIO_ASSERTM(parse(str, result), "Unsafe parsing failed");
    return result;
}

void test_toUpperCase(const char *input, const std::string &expected)
{
    std::string str = input;
    toUpperCase(str);
    VXIO_ASSERT_EQ(str, expected);
}

void test_toLowerCase(const char *input, const std::string &expected)
{
    std::string str = input;
    toLowerCase(str);
    VXIO_ASSERT_EQ(str, expected);
}

enum class SomeEnum : int { A = -12345, B = 0, C = 6789 };

struct SomeStruct {
    unsigned x;
    unsigned y;
};

// maybe_unused because of false detection
// this function is actually used in the universal stringification test
[[maybe_unused]] std::ostream &operator<<(std::ostream &stream, const SomeStruct &s)
{
    return stream << '(' << s.x << ", " << s.y << ')';
}

static_assert(std::is_same_v<std::string, decltype(stringify(SomeEnum{}))>);
static_assert(std::is_same_v<std::string, decltype(stringify(SomeStruct{}))>);

VXIO_TEST(stringify, stringifyFraction)
{
    VXIO_ASSERT_EQ("0.5", stringifyFraction(1u, 2));
    VXIO_ASSERT_EQ("0.3333", stringifyFraction(1u, 3, 4));
    VXIO_ASSERT_EQ("3.3333", stringifyFraction(10u, 3, 4));
    VXIO_ASSERT_EQ("58.82352941176470588235", stringifyFraction(1000u, 17u, 20));
    VXIO_ASSERT_EQ("0.71428571428", stringifyFraction(5u, 7u, 11));

    std::string test1 = stringifyFraction(7u, 4);
    std::string test2 = stringifyFraction(7u, 3);
}

VXIO_TEST(stringify, variousManual)
{
    VXIO_ASSERT_EQ(stringify(false), "false");
    VXIO_ASSERT_EQ(stringify(true), "true");

    VXIO_ASSERT_EQ(stringify(0), "0");
    VXIO_ASSERT_EQ(stringify(1), "1");
    VXIO_ASSERT_EQ(stringify(-12345), "-12345");
    VXIO_ASSERT_EQ(stringify(12345), "12345");

    VXIO_ASSERT_EQ(parseUnsafe<double>(stringify(22.5)), 22.5);
    VXIO_ASSERT_EQ(parseUnsafe<float>(stringify(22.5)), 22.5f);

    VXIO_ASSERT_EQ(stringify(SomeEnum::A), "-12345");
    VXIO_ASSERT_EQ(stringify(SomeEnum::B), "0");
    VXIO_ASSERT_EQ(stringify(SomeEnum::C), "6789");

    VXIO_ASSERT_EQ(stringify("jahgjkshak"), "jahgjkshak");
    VXIO_ASSERT_EQ(stringify(std::string{"jahgjkshak"}), "jahgjkshak");

    VXIO_ASSERT_EQ(stringify(SomeStruct{123, 456}), "(123, 456)");
}

VXIO_TEST(stringify, toHexString_singleDigit)
{
    auto nine = stringifyHex(0x9);

    VXIO_ASSERT_EQ(stringifyHex(0x0), "0");
    VXIO_ASSERT_EQ(stringifyHex(0x1), "1");
    VXIO_ASSERT_EQ(stringifyHex(0x2), "2");
    VXIO_ASSERT_EQ(stringifyHex(0x3), "3");
    VXIO_ASSERT_EQ(stringifyHex(0x4), "4");
    VXIO_ASSERT_EQ(stringifyHex(0x5), "5");
    VXIO_ASSERT_EQ(stringifyHex(0x6), "6");
    VXIO_ASSERT_EQ(stringifyHex(0x7), "7");
    VXIO_ASSERT_EQ(stringifyHex(0x8), "8");
    VXIO_ASSERT_EQ(stringifyHex(0x9), "9");
    VXIO_ASSERT_EQ(stringifyHex(0xa), "a");
    VXIO_ASSERT_EQ(stringifyHex(0xb), "b");
    VXIO_ASSERT_EQ(stringifyHex(0xc), "c");
    VXIO_ASSERT_EQ(stringifyHex(0xd), "d");
    VXIO_ASSERT_EQ(stringifyHex(0xe), "e");
    VXIO_ASSERT_EQ(stringifyHex(0xf), "f");
}

VXIO_TEST(stringify, stringifyHex_manual)
{
    VXIO_ASSERT_EQ(stringifyHex(0x12), "12");
    VXIO_ASSERT_EQ(stringifyHex(0x8839), "8839");
    VXIO_ASSERT_EQ(stringifyHex(0x12345678), "12345678");
}

VXIO_TEST(stringify, stringifyDec_manual)
{
    std::string first = stringifyDec(123);

    VXIO_ASSERT_EQ(stringifyDec(123), "123");
    VXIO_ASSERT_EQ(stringifyDec(456), "456");
    VXIO_ASSERT_EQ(stringifyDec(1234567890), "1234567890");
}

VXIO_TEST(stringify, stringifyBin_oneAndZero)
{
    VXIO_ASSERT_EQ(stringifyBin(0b00), "0");
    VXIO_ASSERT_EQ(stringifyBin(0b01), "1");
}

VXIO_TEST(stringify, stringifyBin_manual)
{
    VXIO_ASSERT_EQ(stringifyBin(0b10), "10");
    VXIO_ASSERT_EQ(stringifyBin(0b11), "11");
    VXIO_ASSERT_EQ(stringifyBin(0b11), "11");
    VXIO_ASSERT_EQ(stringifyBin(0b11001100), "11001100");
    VXIO_ASSERT_EQ(stringifyBin(0b1001010101000001111101010010101010100000100111111111000111ULL),
                   "1001010101000001111101010010101010100000100111111111000111");
    VXIO_ASSERT_EQ(stringifyBin(0b1110110111111011010100011110001011100011111000010010110111100101ULL),
                   "1110110111111011010100011110001011100011111000010010110111100101");
}

VXIO_TEST(stringify, stringifyLargeInt_manual)
{
    VXIO_ASSERT_EQ(stringifyLargeInt(0), "0");
    VXIO_ASSERT_EQ(stringifyLargeInt(999), "999");
    VXIO_ASSERT_EQ(stringifyLargeInt(1000), "1,000");
    VXIO_ASSERT_EQ(stringifyLargeInt(987654, '_'), "987_654");
    VXIO_ASSERT_EQ(stringifyLargeInt(1234567), "1,234,567");
}

VXIO_TEST(stringify, stringifyFileSize_manual)
{
    VXIO_ASSERT_EQ(stringifyFileSize1024(0), "0 B");
    VXIO_ASSERT_EQ(stringifyFileSize1024(1), "1 B");
    VXIO_ASSERT_EQ(stringifyFileSize1024(100), "100 B");
    VXIO_ASSERT_EQ(stringifyFileSize1024(1023), "1023 B");
    VXIO_ASSERT_EQ(stringifyFileSize1024(1024), "1 KiB");
    VXIO_ASSERT_EQ(stringifyFileSize1024(2048), "2 KiB");
    VXIO_ASSERT_EQ(stringifyFileSize1024(1024), "1 KiB");
    VXIO_ASSERT_EQ(stringifyFileSize1024(1024 * 1024), "1 MiB");

    VXIO_ASSERT_EQ((stringifyFileSize1024(1024 * 1024 - 1, 2)), "1023.99 KiB");
    VXIO_ASSERT_EQ((stringifyFileSize1000(999'999, 3)), "999.999 KB");
}

// PARSE ===============================================================================================================

VXIO_TEST(parse, parsesmallIntegrals_manual)
{
    for (schar x : std::initializer_list<signed char>{0, 1, -1, 7, -7}) {
        testParsing<int8_t>(x);
        testParsing<int16_t>(x);
        testParsing<int32_t>(x);
        testParsing<int64_t>(x);
        testParsing<float>(x);
        testParsing<double>(x);
        testParsing<long double>(x);
    }
}

VXIO_TEST(parse, parseZeroAndOne)
{
    for (schar x : std::initializer_list<signed char>{0, 1}) {
        testParsing<int8_t>(x);
        testParsing<int16_t>(x);
        testParsing<int32_t>(x);
        testParsing<int64_t>(x);
        testParsing<uint8_t>(x);
        testParsing<uint16_t>(x);
        testParsing<uint32_t>(x);
        testParsing<uint64_t>(x);
        testParsing<float>(x);
        testParsing<double>(x);
        testParsing<long double>(x);
    }
}

// STRINGMANIP =========================================================================================================

VXIO_TEST(stringmanip, toUpperCase_manual)
{
    test_toUpperCase("Hello World!", "HELLO WORLD!");
    test_toUpperCase("hello world!", "HELLO WORLD!");
    test_toUpperCase("HELLO WORLD!", "HELLO WORLD!");
    test_toUpperCase("~!.-()//%(&", "~!.-()//%(&");
    test_toUpperCase("", "");
}

VXIO_TEST(stringmanip, toLowerCase_manual)
{
    test_toLowerCase("Hello World!", "hello world!");
    test_toLowerCase("hello world!", "hello world!");
    test_toLowerCase("HELLO WORLD!", "hello world!");
    test_toLowerCase("~!.-()//%(&", "~!.-()//%(&");
    test_toLowerCase("", "");
}

constexpr const char *TEST_FILE_PATH_CSTR = "/dir/x/aloah.txt.old";

VXIO_TEST(stringmanip, substrAfterFirst)
{
    const auto actual = substrAfterFirst(TEST_FILE_PATH_CSTR, '/');
    VXIO_ASSERT_EQ(actual, std::string{"dir/x/aloah.txt.old"});
    VXIO_ASSERT_EQ(substrAfterFirst(TEST_FILE_PATH_CSTR, '&'), TEST_FILE_PATH_CSTR);
}

VXIO_TEST(stringmanip, substrAfterLast)
{
    const auto actual = substrAfterLast(TEST_FILE_PATH_CSTR, '/');
    VXIO_ASSERT_EQ(actual, std::string{"aloah.txt.old"});
    VXIO_ASSERT_EQ(substrAfterLast(TEST_FILE_PATH_CSTR, '&'), TEST_FILE_PATH_CSTR);
}

VXIO_TEST(stringmanip, substrBeforeFirst)
{
    const auto actual = substrBeforeFirst(TEST_FILE_PATH_CSTR, '.');
    VXIO_ASSERT_EQ(actual, std::string{"/dir/x/aloah"});
    VXIO_ASSERT_EQ(substrBeforeFirst(TEST_FILE_PATH_CSTR, '&'), TEST_FILE_PATH_CSTR);
}

VXIO_TEST(stringmanip, substrBeforeLast)
{
    const auto actual = substrBeforeLast(TEST_FILE_PATH_CSTR, '.');
    VXIO_ASSERT_EQ(actual, std::string{"/dir/x/aloah.txt"});
    VXIO_ASSERT_EQ(substrBeforeLast(TEST_FILE_PATH_CSTR, '&'), TEST_FILE_PATH_CSTR);
}

}  // namespace
}  // namespace voxelio::test
