#ifndef VXIO_BINVOX_HPP
#define VXIO_BINVOX_HPP

#include "../types.hpp"
#include "../voxelio.hpp"

namespace voxelio::binvox {

class Reader : public AbstractReader {
private:
    struct Header {
        u64 volume;
        Vec3u32 dim;
        Vec3f translation;
        float scale;
        bool dimInitialized = false;
        bool translationInitialized = false;
        bool scaleInitialized = false;
    };

    struct State {
        u64 index = 0;
        u64 lineNum = 0;
        u32 resumeCount = 0;
        u32 readVoxels = 0;
    };

    Header header;
    State state;
    argb32 color = 0xffffffff;
    bool initialized = false;
    u8 voxelBuffer[2];

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;

    /**
     * @brief Sets the color of all following read voxels.
     *
     * Binvox is a format which does not store color information.
     * By default, all voxels will be white, this function allows setting the uniform color of all voxels to a
     * user-chosen color.
     *
     * @param color the color in ARGB format
     */
    void setColor(argb32 color)
    {
        this->color = color;
    }

private:
    // utility
    [[nodiscard]] Vec3u64 posOf(u64 index);

    // header
    [[nodiscard]] ReadResult readMagicAndVersion();
    [[nodiscard]] ReadResult readHeaderFields();
    [[nodiscard]] ReadResult parseHeaderLine(const std::string &line);

    // content
    [[nodiscard]] bool resumeWritingToBuffer(Voxel64 buffer[], size_t bufferLength);
    [[nodiscard]] ReadResult readNextVoxels(Voxel64 buffer[], size_t bufferLength);
};

}  // namespace voxelio::binvox

#endif  // VOXELIO_BINVOX_HPP
