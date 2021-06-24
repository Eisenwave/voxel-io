#ifndef VXIO_BUFSTREAM_HPP
#define VXIO_BUFSTREAM_HPP

#include "stream.hpp"

namespace voxelio {

constexpr usize MIN_STREAM_BUFFER_SIZE = 16;
constexpr usize DEF_STREAM_BUFFER_SIZE = BUFSIZ;

namespace detail {

template <typename ForwardIt, typename T>
constexpr std::size_t indexOf(ForwardIt begin, ForwardIt end, T value)
{
    ForwardIt it = begin;
    for (; it != end && *it != value; ++it) {
    }
    return static_cast<std::size_t>(it - begin);
}

template <usize BUFFER_SIZE>
class BufferedInputStreamBase {
public:
    struct ReadUntilResult {
        u8 *data;
        usize size;
    };

protected:
    static_assert(BUFFER_SIZE >= MIN_STREAM_BUFFER_SIZE);

    u8 buffer[BUFFER_SIZE];
    usize head = 0;
    usize limit = 0;
    InputStream *source;

public:
    BufferedInputStreamBase(InputStream &source) noexcept : source{&source}
    {
        VXIO_DEBUG_ASSERT_NOTNULL(this->source);
    }

    BufferedInputStreamBase(const BufferedInputStreamBase &) = delete;
    BufferedInputStreamBase(BufferedInputStreamBase &&) = delete;

    u8 read()
    {
        VXIO_DEBUG_ASSERT_LE(head, limit);
        if (head == limit) {
            unsafeRefillBuffer();
            u8 result = buffer[head];
            head += limit != 0;
            return result;
        }
        return buffer[head++];
    }

    [[nodiscard]] usize read(u8 out[], usize size)
    {
        VXIO_DEBUG_ASSERT_NOTNULL(out);

        const usize avail = available();
        if (size <= avail) {
            unsafeCopyBuffered(out, size);
            return size;
        }
        else {
            unsafeCopyBuffered(out, available());
            unsafeClearBuffer();
            return avail + unsafeReadFresh(out + avail, size - avail);
        }
    }

    [[nodiscard]] usize readUntil(u8 out[], usize size, u8 delimiter)
    {
        VXIO_DEBUG_ASSERT_NOTNULL(out);

        usize total = 0;
        do {
            usize avail = available();
            if (size <= avail) {
                return total + unsafeCopyBufferedUntil(out, size, delimiter);
            }
            usize copied = unsafeCopyBufferedUntil(out, avail, delimiter);
            if (copied != avail) {
                return total + copied;
            }
            out += copied;
            size -= copied;
            total += copied;
            unsafeRefillBuffer();
        } while (not eof());

        return total;
    }

    /**
     * @brief Reads bytes into an output buffer until a delimiter.
     * The output buffer is dynamically (re-)allocated when needed.
     *
     * If the given size is zero or the delimiter is found at the first index, making the allocation unnecessary, the
     * value of the resulting pointer may be null and the reallocation function may not be called.
     *
     * @tparam Reallocate a function object with the signature void*(void*,std::size_t) which behaves like std::realloc.
     * @param delimiter the delimiter
     * @param realloc the realloaction function
     * @return the final, reallocated result pointer and the final size, or null if no allocation was necessary
     */
    template <typename Reallocate>
    auto readUntil(Reallocate realloc, u8 delimiter)
        -> std::enable_if_t<std::is_invocable_r_v<void *, Reallocate, void *, usize>, ReadUntilResult>
    {
        u8 *result = nullptr;
        usize totalSize = 0;
        do {
            usize avail = available();
            usize scanIndex = detail::indexOf(buffer + head, buffer + limit, delimiter);
            result = static_cast<u8 *>(realloc(result, totalSize + scanIndex));
            unsafeCopyBuffered(result + totalSize, scanIndex);
            totalSize += scanIndex;
            if (scanIndex != avail) {
                head += 1;
                VXIO_ASSERT_LE(head, limit);
                break;
            }
            unsafeRefillBuffer();
        } while (not eof());

        return {result, totalSize};
    }

    template <usize N>
    usize ensureBuffered() noexcept
    {
        static_assert(N < BUFFER_SIZE, "Can't ensure more than the buffer size");
        return unsafeEnsureBufferSize(N);
    }

    usize available() const noexcept
    {
        VXIO_DEBUG_ASSERT_LE(head, limit);
        return limit - head;
    }

    u64 position() noexcept
    {
        return source->position() - limit + head;
    }

    void seekAbsolute(u64 index) noexcept
    {
        u64 sourcePosition = source->position();
        u64 position = sourcePosition - limit + head;
        if (i64 remaining = seekBufferRelative(static_cast<i64>(index - position))) {
            source->seekAbsolute(sourcePosition + remaining);
        }
    }

    void seekRelative(i64 offset) noexcept
    {
        if (i64 remaining = seekBufferRelative(offset)) {
            source->seekRelative(remaining);
        }
    }

    void clearErrors() noexcept
    {
        source->clearErrors();
    }

    bool eof() const noexcept
    {
        VXIO_DEBUG_ASSERT_LE(head, limit);
        return head == 0 && limit == 0 && source->eof();
    }

    bool err() const noexcept
    {
        return source->err();
    }

    bool good() const noexcept
    {
        return not(eof() || err());
    }

protected:
    /**
     * @brief Seeks relatively within the buffer.
     * @param offset the offset from the current read head position
     * @return the remaining difference on disk, after seeking to the lower or upper limit of the buffer
     */
    i64 seekBufferRelative(i64 offset) noexcept
    {
        auto newHead = static_cast<i64>(head) + offset;

        if (newHead < 0 || newHead > static_cast<i64>(limit)) {
            i64 result = newHead - static_cast<i64>(limit);
            head = limit = 0;
            return result;
        }
        else {
            head = static_cast<u64>(newHead);
            return 0;
        }
    }

    /**
     * @brief Unsafe operation.
     * Copies a given amount of bytes from the internal buffer into an output buffer.
     * The amount of bytes must not be greater than the number of available bytes.
     * @param out the output buffer
     * @param size the number of bytes to copy
     */
    void unsafeCopyBuffered(u8 out[], usize size) noexcept
    {
        VXIO_DEBUG_ASSERT_NOTNULL(out);
        VXIO_DEBUG_ASSERT_LE(size, available());

        std::memcpy(out, buffer + head, size);
        head += size;
    }

    /**
     * @brief Unsafe operation.
     * Copies a given amount of bytes from the internal buffer into an output buffer, or less if a delimiter is found
     * in the buffered bytes.
     * @param out the output buffer
     * @param size the maximum number of bytes to copy
     * @param delimiter the delimiter byte
     * @return the actual number of byte copied, which is < size when a delimiter was found
     */
    usize unsafeCopyBufferedUntil(u8 out[], usize size, u8 delimiter)
    {
        VXIO_DEBUG_ASSERT_NOTNULL(out);
        VXIO_DEBUG_ASSERT_LE(size, available());

        usize i = detail::indexOf(buffer + head, buffer + head + size, delimiter);
        std::memcpy(out, buffer + head, i);
        head += i;
        head += size != i;  // extract delimiter, if found
        VXIO_DEBUG_ASSERT_LE(head, limit);
        return i;
    }

    usize unsafeReadFresh(u8 out[], usize size) noexcept
    {
        VXIO_DEBUG_ASSERT_NOTNULL(out);
        VXIO_DEBUG_ASSERT_EQ(head, 0u);
        VXIO_DEBUG_ASSERT_EQ(limit, 0u);

        usize bigTotal = 0;
        if (size > BUFFER_SIZE) {
            bigTotal = size - size % BUFFER_SIZE;
            VXIO_DEBUG_ASSERT_NE(bigTotal, 0u);

            if (usize actual = source->read(out, bigTotal); actual != bigTotal) {
                return actual;
            }
            out += bigTotal;
            size -= bigTotal;
        }
        VXIO_DEBUG_ASSERT_LE(size, BUFFER_SIZE);

        limit = source->read(buffer, BUFFER_SIZE);
        if (size > limit) {
            std::memcpy(out, buffer, limit);
            bigTotal += limit;
            head = limit = 0;
        }
        else {
            std::memcpy(out, buffer, size);
            head = size;
            bigTotal += size;
        }
        return bigTotal;
    }

    /**
     * @brief Unsafe operation.
     * Sets both the limit and the head to zero, indicating an empty buffer with no bytes left.
     */
    void unsafeClearBuffer() noexcept
    {
        VXIO_DEBUG_ASSERT_EQ(head, limit);

        limit = 0;
        head = 0;
    }

    /**
     * @brief Unsafe operation.
     * Refills the buffer completely.
     * After this operation, the head will be set to zero and unless EOF occurred, limit will be set to BUFFER_SIZE.
     */
    void unsafeRefillBuffer() noexcept
    {
        VXIO_DEBUG_ASSERT_EQ(head, limit);

        limit = source->read(buffer, BUFFER_SIZE);
        head = 0;
    }

    /**
     * @brief Unsafe operation.
     * Ensures that unless EOF is reached, a desired amount of bytes will be present in the buffer.
     * If there are enough bytes left in the buffer, this is a no-op.
     * Otherwise, the head will be moved to the start and at least the ensured amount of bytes will be buffered.
     * @param ensured the number of ensured bytes; must be <= BUFFER_SIZE
     */
    usize unsafeEnsureBufferSize(usize ensured)
    {
        VXIO_DEBUG_ASSERT_LE(ensured, BUFFER_SIZE);

        std::size_t avail = available();
        if (avail >= ensured) {
            return avail;
        }
        unsafeMoveBytesToStart();
        limit += source->read(buffer + limit, BUFFER_SIZE - limit);
        return std::min(ensured, limit);
    }

    /**
     * @brief Unsafe operation. Moves all remaining bytes in the buffer to the start by copying.
     * The head will be equal to zero.
     */
    void unsafeMoveBytesToStart() noexcept
    {
        std::size_t avail = available();
        std::memmove(buffer, buffer + head, avail);
        head = 0;
        limit = avail;
        VXIO_DEBUG_ASSERT_EQ(avail, available());
    }
};

}  // namespace detail

// BUFFERED INPUT STREAM ===============================================================================================

template <std::size_t BUFFER_SIZE = DEF_STREAM_BUFFER_SIZE>
struct BufferedInputStream : public detail::BufferedInputStreamBase<BUFFER_SIZE> {
public:
    BufferedInputStream(InputStream &stream) : detail::BufferedInputStreamBase<BUFFER_SIZE>{stream} {}

    /**
     * @brief Reads an unsigned 8-bit integer.
     * May set eof and err.
     * @return the read data
     */
    u8 readU8()
    {
        return readData<u8, Endian::NATIVE>();
    }

    /**
     * @brief Reads a signed 8-bit integer.
     * May set eof and err.
     * @return the read data
     */
    i8 readI8()
    {
        return readData<i8, Endian::NATIVE>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T>
    auto readBig() -> std::enable_if_t<isIoType<T>, T>
    {
        return readData<T, Endian::BIG>();
    }

    /**
     * @brief Reads an unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T>
    auto readLittle() -> std::enable_if_t<isIoType<T>, T>
    {
        return readData<T, Endian::LITTLE>();
    }

    /**
     * @brief Reads an unsigned, native-endian arithmetic type.
     * May set eof and err.
     * @return the read data
     */
    template <typename T>
    auto readNative() -> std::enable_if_t<isIoType<T>, T>
    {
        return readData<T, Endian::NATIVE>();
    }

    /**
     * @brief Reads multiple unsigned, little-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T>
    auto readLittle(T t[COUNT]) -> std::enable_if_t<isIoType<T>, void>
    {
        return readData<T, Endian::LITTLE>(t, COUNT);
    }

    /**
     * @brief Reads multiple unsigned, big-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T>
    auto readBig(T t[COUNT]) -> std::enable_if_t<isIoType<T>, void>
    {
        return readData<T, Endian::BIG>(t, COUNT);
    }

    /**
     * @brief Reads multiple unsigned, native-endian arithmetic type.
     * May set eof and err.
     * @param t the buffer for read data
     */
    template <usize COUNT, typename T>
    auto readNative(T t[COUNT]) -> std::enable_if_t<isIoType<T>, void>
    {
        return readData<T, Endian::NATIVE>(t, COUNT);
    }

    void readStringUntil(std::string &dest, u8 delimiter)
    {
        auto realloc = [&dest](void *, std::size_t size) -> void * {
            dest.resize(size, '\0');
            return dest.data();
        };
        this->readUntil(realloc, delimiter);
    }

    std::string readStringUntil(u8 delimiter)
    {
        std::string result;
        readStringUntil(result, delimiter);
        return result;
    }

    void readString(std::string &dest, usize size)
    {
        dest.resize(size);
        usize actual = this->read(reinterpret_cast<u8 *>(dest.data()), size);
        dest.resize(actual);
    }

    std::string readString(usize size)
    {
        std::string result;
        readString(result, size);
        return result;
    }

private:
    template <typename T, Endian ENDIAN>
    [[nodiscard]] T readData()
    {
        if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(readData<std::underlying_type_t<T>, ENDIAN>());
        }
        else if constexpr (sizeof(T) == 1) {
            return static_cast<T>(this->read());
        }
        else {
            this->template ensureBuffered<sizeof(T)>();
            T result = decode<ENDIAN, T>(this->buffer + this->head);
            this->head += sizeof(T);
            return result;
        }
    }

    template <typename T, Endian ENDIAN>
    usize readData(T out[], usize size)
    {
        if constexpr (std::is_enum_v<T>) {
            this->readData<std::underlying_type_t<T>, ENDIAN>(out, size);
        }
        else {
            u8 *outBytes = reinterpret_cast<u8 *>(out);
            usize actual = this->read(outBytes, size * sizeof(T)) / sizeof(T);
            if constexpr (sizeof(T) != 1 && ENDIAN != Endian::NATIVE) {
                for (usize i = 0; i < actual; ++i) {
                    encode<ENDIAN, T>(out[i], outBytes + i * sizeof(T));
                }
            }
            return actual;
        }
    }
};

template <usize BUFFER_SIZE = DEF_STREAM_BUFFER_SIZE>
BufferedInputStream(InputStream &)->BufferedInputStream<BUFFER_SIZE>;

}  // namespace voxelio

#endif  // BUFSTREAM_HPP
