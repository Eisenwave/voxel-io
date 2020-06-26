#ifndef VXIO_IOUTIL_HPP
#define VXIO_IOUTIL_HPP

#include "types.hpp"
#include "voxelio.hpp"

#include <cstddef>

namespace voxelio {

/**
 * @brief Simplifies writing either 32-bit or 64-bit voxels to a buffer.
 *
 * This class manages either a 32-bit or 64-bit buffer in a type-safe way.
 * The caller can simply write a Voxel32 or Voxel64 to this write WriteHelper without having to worry about the current
 * format.
 *
 * Voxel32 is expanded to a Voxel64 if necessary and a Voxel64 is narrowed to a Voxel32 respectively.
 */
class VoxelBufferWriteHelper {
private:
    union {
        Voxel32 *buffer32;
        Voxel64 *buffer64;
    };
    size_t index = 0;
    size_t limit = 0;
    bool is64;

public:
    void reset(Voxel32 buffer[], size_t size) noexcept;
    void reset(Voxel64 buffer[], size_t size) noexcept;

    void emplace(Vec3i32 pos, u32 color) noexcept;
    void emplace(Vec3i64 pos, u32 color) noexcept;

    void write(Voxel32 voxel) noexcept;
    void write(Voxel64 voxel) noexcept;

    bool canWrite() noexcept
    {
        return index != limit;
    }

    bool isFull() noexcept
    {
        return index == limit;
    }

    size_t voxelsWritten() noexcept
    {
        return index;
    }

    size_t capacity() noexcept
    {
        return limit;
    }
};

/**
 * A utility wrapper class for AbstractListWriter simplifies writing individual voxels.
 * AbstractListWriters require buffers of voxels to be passed to them and would not perform well for small buffers.
 * The BufferedListWriter stores written voxels in an unowned buffer and flushes them whenever necessary and upon
 * destruction.
 */
template <typename Voxel, std::enable_if_t<is_voxel_v<Voxel>, int> = 0>
class ListWriterWriteHelper {
private:
    AbstractListWriter &writer;
    Voxel *buffer;
    size_t bufferIndex = 0;
    size_t bufferSize = 0;

public:
    ListWriterWriteHelper(AbstractListWriter &writer, Voxel buffer[], size_t bufferSize)
        : writer{writer}, buffer{std::move(buffer)}, bufferSize{bufferSize}
    {
    }

    ~ListWriterWriteHelper()
    {
        auto result = flush();
        if (isError(result)) {
            ALWAYS_ASSERTM(false, std::string{"flush() in destructor produced bad result code: "} + nameOf(result));
        }
    }

    [[nodiscard]] ResultCode write(Voxel buffer[], size_t bufferSize) noexcept
    {
        for (size_t i = 0; i < bufferSize; ++i) {
            if (auto result = write(std::move(buffer[i])); isError(result)) {
                return result;
            }
        }
        return ResultCode::WRITE_OK;
    }

    [[nodiscard]] ResultCode write(Voxel voxel)
    {
        VXIO_DEBUG_ASSERT_NE(bufferSize, 0);
        auto result = bufferIndex == bufferSize ? flush() : ResultCode::WRITE_BUFFER_UNDERFULL;
        buffer[bufferIndex++] = std::move(voxel);
        return result;
    }

    [[nodiscard]] ResultCode write(typename Voxel::pos_t pos, u32 color)
    {
        return write({pos, {color}});
    }

    [[nodiscard]] ResultCode flush() noexcept
    {
        auto result = writer.write(buffer, bufferIndex);
        bufferIndex = 0;
        return result;
    }
};

using ListWriterWriteHelper32 = ListWriterWriteHelper<Voxel32>;
using ListWriterWriteHelper64 = ListWriterWriteHelper<Voxel64>;

}  // namespace voxelio

#endif  // VXIO_UTIL_PUBLIC_HPP
