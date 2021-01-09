#ifndef VXIO_QEF_HPP
#define VXIO_QEF_HPP

#include "voxelio/ioutil.hpp"
#include "voxelio/stream.hpp"
#include "voxelio/types.hpp"
#include "voxelio/voxelarray.hpp"
#include "voxelio/voxelio.hpp"

namespace voxelio {
namespace qef {

class Reader : public AbstractReader {
private:
    size_t paletteSize;
    size_t paletteIndex = 0;
    std::unique_ptr<argb32[]> palette;
    Vec3u64 dimensions;
    VoxelBufferWriteHelper writeHelper;
    bool initialized = false;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;
    [[nodiscard]] ReadResult read(Voxel32 buffer[], size_t bufferLength);

private:
    [[nodiscard]] ReadResult doRead() noexcept;
    [[nodiscard]] ReadResult readLine(std::string &out) noexcept;
    [[nodiscard]] ReadResult parseLine(size_t num, const std::string &line) noexcept;
    [[nodiscard]] ReadResult parseDimensions(const std::string &line) noexcept;
    [[nodiscard]] ReadResult parseColorCount(const std::string &line) noexcept;
    [[nodiscard]] ReadResult parseColorDefinition(u64 num, const std::string &line) noexcept;
    [[nodiscard]] ReadResult parseVoxelDefinition(u64 num, const std::string &line) noexcept;
};

class Writer : public AbstractListWriter {
private:
    bool initialized = false;

public:
    Writer(OutputStream &ostream) : AbstractListWriter{ostream} {}

    [[nodiscard]] ResultCode init() noexcept override;
    [[nodiscard]] ResultCode write(Voxel32 buffer[], size_t bufferLength) noexcept override;

private:
    [[nodiscard]] ResultCode writeString(std::string line) noexcept;
    [[nodiscard]] ResultCode writePalette() noexcept;
    [[nodiscard]] ResultCode writeColorLine(Color32 color) noexcept;
    [[nodiscard]] ResultCode writeVoxelLine(Voxel32 v) noexcept;
    [[nodiscard]] ResultCode verifyVoxel(Voxel32 voxel) noexcept;
};

/**
 * <p>
 *     A parser for <b>Qubicle Exchange Format (.qef)</b> files.
 * </p>
 * <p>
 *     Only version <b>2.0</b> is supported.
 * </p>
 * Qubicle Geometry note:<ul>
 *     <li>y-axis points upwards</li>
 *     <li>z-axis 90 degrees to the right of x-axis</li>
 * </ul>
 */
class Deserializer : public AbstractDeserializer {
private:
    std::optional<VoxelArray> voxels;
    std::vector<Color32> colors;
    size_t colorCount;

public:
    Deserializer(InputStream &istream) : AbstractDeserializer{istream} {}

    VoxelArray read() noexcept(false);

private:
    void parseLine(size_t num, const std::string &line) noexcept(false);
    void parseDimensions(const std::string &line) noexcept(false);
    size_t parseColorCount(const std::string &line) noexcept(false);
    void parseColorDefinition(size_t num, const std::string &line) noexcept(false);
    void parseVoxelDefinition(size_t num, const std::string &line) noexcept(false);
};

}  // namespace qef
}  // namespace voxelio

#endif  // QUBICLE_EXCHANGE_FORMAT_HPP
