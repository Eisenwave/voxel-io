#include "voxelio/bufstream.hpp"

#include "voxelio/test/random.hpp"
#include "voxelio/test/test.hpp"

#include <array>

namespace voxelio::test {

VXIO_TEST(bufstream, bytesReadOneByOne)
{
    constexpr u8 data[]{0x00, 0x11, 0x22, 0x33, 0x6f, 0x77, 0x00, 0x54};
    ByteArrayInputStream stream{data, sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    for (u8 expected : data) {
        VXIO_ASSERT_EQ(bstream.read(), expected);
        VXIO_ASSERT(bstream.good());
    }
    bstream.read();
    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
    VXIO_ASSERT(not bstream.good());

    bstream.read();
    VXIO_ASSERT(bstream.eof());
}

VXIO_TEST(bufstream, bytesReadOneByOneWithBuffer)
{
    constexpr u8 data[]{0x00, 0x11, 0x22, 0x33, 0x6f, 0x77, 0x00, 0x54};
    ByteArrayInputStream stream{data, sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    for (u8 expected : data) {
        u8 buffer;
        VXIO_ASSERT_EQ(bstream.read(&buffer, 1), 1u);
        VXIO_ASSERT_EQ(buffer, expected);
        VXIO_ASSERT(bstream.good());
    }
    bstream.read();
    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
}

VXIO_TEST(bufstream, bytesReadFour)
{
    using arr4b = std::array<u8, 4>;

    constexpr std::array<u8, 8> data{0x00, 0x11, 0x22, 0x33, 0x6f, 0x77, 0x00, 0x54};
    ByteArrayInputStream stream{data.data(), sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    arr4b part;
    VXIO_ASSERT_EQ(bstream.read(part.data(), sizeof(part)), 4u);

    VXIO_ASSERT(part == arr4b{0x00, 0x11, 0x22, 0x33});
    VXIO_ASSERT_EQ(bstream.read(part.data(), sizeof(part)), 4u);
    VXIO_ASSERT(part == arr4b{0x6f, 0x77, 0x00, 0x54});

    VXIO_ASSERT(bstream.good());
    VXIO_ASSERT_EQ(bstream.read(part.data(), sizeof(part)), 0u);

    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
}

VXIO_TEST(bufstream, bytesReadPartialFresh)
{
    using arr4b = std::array<u8, 4>;

    constexpr std::array<u8, 2> data{0x00, 0x11};
    ByteArrayInputStream stream{data.data(), sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    arr4b part;
    VXIO_ASSERT_EQ(bstream.read(part.data(), sizeof(part)), 2u);

    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
}

VXIO_TEST(bufstream, bytesReadPartial)
{
    using arr4b = std::array<u8, 4>;

    constexpr std::array<u8, 2> data{0x12, 0x34};
    ByteArrayInputStream stream{data.data(), sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.read(), 0x12u);
    VXIO_ASSERT(bstream.good());

    arr4b part;
    VXIO_ASSERT_EQ(bstream.read(part.data(), sizeof(part)), 1u);

    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
}

VXIO_TEST(bufstream, readLineFresh)
{
    static constexpr u8 data[] = "The quick brown fox jumps over the lazy\n dog";
    u8 out[sizeof(data)];

    ByteArrayInputStream stream{data, sizeof(data)};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    usize read = bstream.readUntil(out, 8, '\n');
    VXIO_ASSERT_EQ(read, 8u);
    VXIO_ASSERT(bstream.good());

    read = bstream.readUntil(out + 8, 1024, '\n');
    VXIO_ASSERT_LT(read, 1024u);
    VXIO_ASSERT(bstream.good());

    out[read + 8] = '\n';
    read = bstream.readUntil(out + read + 8 + 1, 1024, '\n');
    VXIO_ASSERT_EQ(read, 5u);

    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());

    VXIO_ASSERT(std::equal(data, std::end(data), out));

    VXIO_ASSERT_EQ(bstream.readUntil(out, 1024, '\n'), 0u);
    VXIO_ASSERT_EQ(bstream.readUntil(out, 1024, '\n'), 0u);
    VXIO_ASSERT(bstream.eof());
}

VXIO_TEST(bufstream, readStringLineFresh)
{
    static constexpr u8 data[] = "The quick brown \nfox jumps over the lazy\n dog";

    ByteArrayInputStream stream{data, sizeof(data) - 1};
    BufferedInputStream bstream{stream};
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), "The quick brown ");
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), "fox jumps over the lazy");
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), " dog");
    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), "");
    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), "");
    VXIO_ASSERT(bstream.eof());
}

VXIO_TEST(bufstream, readLongStringLine)
{
    static constexpr u8 data[] = "The quick brown brown brown brown brown fox jumps over the\n very very lazy dog\n";

    ByteArrayInputStream stream{data, sizeof(data) - 1};
    BufferedInputStream<MIN_STREAM_BUFFER_SIZE + 1> bstream{stream};
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), "The quick brown brown brown brown brown fox jumps over the");
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT_EQ(bstream.readStringUntil('\n'), " very very lazy dog");
    VXIO_ASSERT(bstream.good());

    VXIO_ASSERT(bstream.readStringUntil('\n').empty());

    VXIO_ASSERT(bstream.eof());
    VXIO_ASSERT(not bstream.err());
}

VXIO_TEST(bufstream, readRandomInput)
{
    u8 expected[1024 * 16];
    makeRandomData(expected, sizeof(expected));

    u8 actual[sizeof(expected)];
    usize index = 0;

    ByteArrayInputStream stream{expected, sizeof(expected)};
    BufferedInputStream<MIN_STREAM_BUFFER_SIZE + 1> bstream{stream};
    VXIO_ASSERT(bstream.good());

    while (not bstream.eof()) {
        actual[index] = bstream.read();
        if (bstream.eof()) {
            ++index;
            break;
        }
        index += bstream.read(actual + index + 1, actual[index]);
        index += 1;
    }

    VXIO_ASSERT_EQ(index, sizeof(expected));
    VXIO_ASSERT(std::equal(expected, std::end(expected), actual));
}

template <bool RELATIVE>
static void testSeekAndReadBytesShuffle()
{
    u8 expected[1024 * 8];
    makeRandomData(expected, sizeof(expected));

    std::size_t indices[sizeof(expected)];
    makeRandomIndexSequence(indices, std::size(indices), 12345);

    ByteArrayInputStream stream{expected, sizeof(expected)};
    BufferedInputStream<256> bstream{stream};
    VXIO_ASSERT(bstream.good());

    u64 prevIndex = 0;
    for (u64 index : indices) {
        if constexpr (RELATIVE) {
            i64 offset = static_cast<i64>(index - prevIndex);
            bstream.seekRelative(offset);
            prevIndex = index + 1;  // + 1 because we advance one by reading
        }
        else {
            bstream.seekAbsolute(index);
        }
        VXIO_ASSERT_EQ(expected[index], bstream.read());
    }
}

VXIO_TEST(bufstream, seekAndReadBytesShuffledAbsolute)
{
    return testSeekAndReadBytesShuffle<false>();
}

VXIO_TEST(bufstream, seekAndReadBytesShuffledRelative)
{
    return testSeekAndReadBytesShuffle<true>();
}

}  // namespace voxelio::test
