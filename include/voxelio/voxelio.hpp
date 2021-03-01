#ifndef VXIO_VOXELIO_HPP
#define VXIO_VOXELIO_HPP

#include "palette.hpp"
#include "results.hpp"
#include "stream.hpp"
#include "types.hpp"

namespace voxelio {

class AbstractReader {
public:
    static constexpr u64 DATA_LENGTH_UNKNOWN = 0;

protected:
    InputStream &stream;
    u64 dataLength;
    Error err;

public:
    AbstractReader(InputStream &stream, u64 dataLength = DATA_LENGTH_UNKNOWN) noexcept;
    virtual ~AbstractReader() noexcept = default;

    /**
     * Returns the last error result.
     * The Error is only initialized, if a bad ReadResult was actually returned.
     * Otherwise accessing the error is undefined behavior.
     */
    [[nodiscard]] const Error &error() const
    {
        return err;
    }

    /**
     * Reads the header information of the data format.
     * The reader is implicitly initialized when reading voxels, so calling this method is not necessary unless header
     * information has to be extracted before voxels are read.
     */
    [[nodiscard]] virtual ReadResult init() noexcept = 0;

    /**
     * Reads <bufferLength> voxels into the given buffer.
     * If the reader has not been explicitly initialized using init() yet, this method will automatically initialize it
     * upon first invocation.
     *
     * It is not guaranteed that the entire buffer will be filled by this, so the user has to verify the exact amount
     * using the return result.
     *
     * @param buffer the output buffer into which all voxels are written
     * @param bufferLength the length of the buffer
     */
    [[nodiscard]] virtual ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept = 0;

    /**
     * Returns an estimate of the progress of the reading progress in [0, 1] where 0 is at the start and 1 means
     * completed.
     * If no estimate can be made, NaN is returned instead.
     * @return the progress in [0, 1] or NaN
     */
    [[nodiscard]] virtual float progress() noexcept;
};

enum class IoState : u8 { UNINITIALIZED, INITIALIZED, FINALIZED };

/**
 * @brief A generic writer for voxel list based formats like QEF or BINVOX or VOBJ with LIST data format.
 */
class AbstractListWriter {
protected:
    OutputStream &stream;
    Error err{};
    voxelio::Palette32 pal;
    std::optional<Vec3u32> globalDims = std::nullopt;
    IoState state = IoState::UNINITIALIZED;

public:
    AbstractListWriter(OutputStream &stream) noexcept;

    virtual ~AbstractListWriter() noexcept = default;

    /**
     * Writes the header of the data format.
     * The writer is implicitly initialized when reading voxels, so calling this method is before writing is optional.
     */
    [[nodiscard]] virtual ResultCode init() noexcept = 0;

    /**
     * Writes <bufferLength> voxels into the stream..
     * If the writer has not been explicitly initialized using init() yet, this method will automatically initialize it
     * upon first invocation.
     *
     * It is guaranteed that the entire buffer will be written, unless an error occurs during this process.
     *
     * @param buffer the input buffer from which all voxels are read
     * @param bufferLength the length of the buffer
     */
    [[nodiscard]] virtual ResultCode write(Voxel32 buffer[], size_t bufferLength) noexcept = 0;

    /**
     * @brief Flushes the writer.
     * This method writes through any remaining changes to the underlying stream.
     * It is safe to invoke this method repeatedly and it must be invoked by the destructor if overridden.
     * However, after finalize() is called, neither write() nor init() may be called.
     * @return the result code
     */
    [[nodiscard]] virtual ResultCode finalize() noexcept
    {
        return ResultCode::OK;
    }

    /**
     * @brief Sets the current canvas dimensions.
     * @param dims the dimensions (must not be zero)
     * @return the result code
     */
    [[nodiscard]] virtual ResultCode setGlobalVolumeSize(Vec3u32 dims) noexcept
    {
        if (isInitialized()) {
            return ResultCode::USER_ERROR_SETTING_VOLUME_SIZE_AFTER_INIT;
        }
        if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0) {
            return ResultCode::USER_ERROR_ILLEGAL_VOLUME_SIZE;
        }
        globalDims = dims;
        return ResultCode::OK;
    }

    /**
     * @brief Sets the dimensions of sub-volumes.
     * Some formats such as VOX have a limited volume size and the voxels must be sorted into multiple independent
     * volumes instead.
     * Sub-volumes are cubical cells in a regular grid with the given size.
     * @param size the sub-volume size in each dimension (must not be zero)
     * @return true if the size is valid and compatible with the writer's format
     */
    [[nodiscard]] virtual ResultCode setSubVolumeSize(u32 size) noexcept
    {
        if (isInitialized()) {
            return ResultCode::USER_ERROR_SETTING_VOLUME_SIZE_AFTER_INIT;
        }
        return size != 0 ? ResultCode::OK : ResultCode::USER_ERROR_ILLEGAL_VOLUME_SIZE;
    }

    // impl

    bool isInitialized() const
    {
        return state != IoState::UNINITIALIZED;
    }

    bool isFinalized() const
    {
        return state == IoState::FINALIZED;
    }

    /**
     * Returns the last error result.
     * The Error is only initialized, if a bad WriteResult was actually returned.
     */
    const Error &error() const
    {
        return err;
    }

    const Palette32 &palette() const
    {
        return pal;
    }

    Palette32 &palette()
    {
        return pal;
    }
};

/// Generic serializer class which writes all voxels to a stream at once.
class AbstractSerializer {
protected:
    OutputStream &streamWrapper;
    Error err;

public:
    AbstractSerializer(OutputStream &stream) : streamWrapper{stream} {}

    const Error &error() const
    {
        return err;
    }
};

/// Generic deserializer class which reads all voxels from a stream at once.
class AbstractDeserializer {
protected:
    InputStream &streamWrapper;
    Error err;

public:
    AbstractDeserializer(InputStream &stream) : streamWrapper{stream} {}

    const Error &error() const
    {
        return err;
    }
};

}  // namespace voxelio

#endif  // VXIO_VOXELIO_HPP
