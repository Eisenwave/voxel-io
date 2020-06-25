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

/**
 * A generic writer for voxel list based formats like QEF or BINVOX or VOBJ with LIST data format.
 */
class AbstractListWriter {
protected:
    OutputStream &stream;
    Error err{};
    voxelio::Palette32 pal;
    std::optional<Vec3u32> canvasDims;

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

    // impl

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

    /**
     * @brief Returns the current canvas dimensions, if set. Fails otherwise.
     * @return the canvas dimensions
     */
    Vec3u32 getCanvasDimensions() const;

    /**
     * @brief Sets the current canvas dimensions.
     * @param dims the dimensions
     * @return true if the canvas dimensions were valid and compatible with the writer's format
     */
    bool setCanvasDimensions(Vec3u32 dims);
};

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
