#ifndef VXIO_QB_HPP
#define VXIO_QB_HPP

#include "voxelio/stream.hpp"
#include "voxelio/voxelarray.hpp"
#include "voxelio/voxelio.hpp"

#include <tuple>
#include <vector>

namespace voxelio {
namespace qb {

enum class QBVersion : u32 { CURRENT = 0x01010000 };

enum class ColorFormat : u32 { RGBA = 0, BGRA = 1 };

enum class ZOrient : u32 { LEFT = 0, RIGHT = 1 };

enum class Compressed : u32 { FALSE = 0, TRUE = 1 };

enum class VisMaskEncoded : u32 { FALSE = 0, TRUE = 1 };

enum class CompressionFlags : u32 { CODEFLAG = 2, NEXTSLICEFLAG = 6 };

struct Header {
    ColorFormat colorFormat;
    u32 numMatrices;
    bool compressed, visibilityMaskEncoded, zLeft;
};

struct MatrixHeader {
    std::string name;
    Vec3i32 pos;
    Vec3u32 size;

    std::pair<Vec3i32, Vec3i32> bounds() const
    {
        auto max = pos + size.cast<i32>() - Vec3i32{1, 1, 1};
        return {pos, max};
    }

    bool operator==(const MatrixHeader &other) const
    {
        return this->name == other.name && this->pos == other.pos && this->size == other.size;
    }
};

struct Matrix : public MatrixHeader {
    VoxelArray voxels;

    Matrix(std::string name, Vec3i32 pos, VoxelArray voxels);

    Matrix(const Matrix &) = default;
    Matrix(Matrix &&) = default;

    std::string toString() const
    {
        return "Matrix{name=" + name + ",voxels=" + voxels.toString() + "}";
    }

    explicit operator std::string() const
    {
        return toString();
    }

    bool operator==(const Matrix &other) const
    {
        return this->MatrixHeader::operator==(other) && this->voxels == other.voxels;
    }
};

class Model {
private:
    std::vector<Matrix> _matrices;

public:
    Model() noexcept = default;
    Model(const Model &) = default;
    Model(Model &&) = default;

    explicit Model(std::vector<Matrix> matrices) : _matrices{std::move(matrices)} {}
    explicit Model(Matrix matrix) : _matrices{matrix} {}

    // GETTERS

    bool empty() const
    {
        return _matrices.empty();
    }

    /**
     * Returns the amount of matrices in this model.
     *
     * @return the matrix count
     */
    size_t matrixCount() const
    {
        return _matrices.size();
    }

    /**
     * Returns the combined volume of all voxel arrays in this mesh.
     *
     * @return the combined volume of all arrays
     */
    size_t computeCombinedVolume() const;

    /**
     * Returns the total amount of voxels in this mesh.
     *
     * @return the voxel count
     */
    size_t countVoxels() const;

    /**
     * Returns the boundaries of this model.
     *
     * @return the model boundaries
     */
    std::pair<Vec3i32, Vec3i32> findBoundaries() const;

    std::vector<Matrix> &matrices()
    {
        return _matrices;
    }

    const std::vector<Matrix> &matrices() const
    {
        return _matrices;
    }

    // MUTATORS

    /**
     * Adds a matrix to this model.
     *
     * @param matrix the matrix
     */
    void add(const Matrix &matrix)
    {
        matrices().push_back(matrix);
    }

    /**
     * Removes all matrices from this model.
     */
    void clear()
    {
        matrices().clear();
    }

    // ITERATION

    std::vector<Matrix>::iterator begin()
    {
        return _matrices.begin();
    }

    std::vector<Matrix>::iterator end()
    {
        return _matrices.end();
    }

    std::vector<Matrix>::const_iterator begin() const
    {
        return _matrices.begin();
    }

    std::vector<Matrix>::const_iterator end() const
    {
        return _matrices.end();
    }

    // MISC

    std::string toString() const
    {
        return "QBModel{size:" + stringify(matrixCount()) + "}";
    }

    explicit operator std::string() const
    {
        return toString();
    }

    bool operator==(const Model &other) const
    {
        return this->_matrices == other._matrices;
    }
};

/**
 * <p>
 *     A deserializer for <b>Qubicle Binary (.qb)</b> files.
 * </p>
 * <p>
 *     No version restrictions exist.
 * </p>
 * Qubicle Geometry note:<ul>
 *     <li>y-axis points upwards</li>
 *     <li>z-axis 90 degrees to the right of x-axis</li>
 * </ul>
 */
class Reader : public AbstractReader {
private:
    Header header;
    unsigned matrixIndex = 0;
    std::string matrixName;
    u32 matSizeX, matSizeY, matSizeZ, matVolume;
    i32 matPosX, matPosY, matPosZ;
    u32 x, y, slice;
    bool initialized = false;
    u64 index;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;

private:
    [[nodiscard]] ReadResult deserializeHeader_version() noexcept;
    [[nodiscard]] ReadResult deserializeHeader_colorFormat() noexcept;
    [[nodiscard]] ReadResult deserializeHeader_zOrient() noexcept;
    [[nodiscard]] ReadResult deserializeHeader_compression() noexcept;
    [[nodiscard]] ReadResult deserializeHeader_visMaskEncoded() noexcept;
    [[nodiscard]] ReadResult deserializeHeader() noexcept;

    [[nodiscard]] ReadResult deserializeMatrixHeaderName() noexcept;
    [[nodiscard]] ReadResult deserializeMatrixHeader() noexcept;

    [[nodiscard]] ReadResult readUncompressed(Voxel64 buffer[], size_t bufferLength) noexcept;
    [[nodiscard]] ReadResult readCompressed(Voxel64 buffer[], size_t bufferLength) noexcept;
    [[nodiscard]] std::pair<u32, u32> readCompressed_writeToBuffer(
        Voxel64 buffer[], size_t bufferLength, size_t bufferIndex, size_t relZ, u32 data, size_t count) noexcept;
};

/**
 * <p>
 *     A deserializer for <b>Qubicle Binary (.qb)</b> files.
 * </p>
 * <p>
 *     No version restrictions exist.
 * </p>
 * Qubicle Geometry note:<ul>
 *     <li>y-axis points upwards</li>
 *     <li>z-axis 90 degrees to the right of x-axis</li>
 * </ul>
 */
class Deserializer : public AbstractDeserializer {
private:
    Header header;

public:
    Deserializer(InputStream &istream) : AbstractDeserializer{istream} {}

    Model read() noexcept(false);

private:
    void deserializeHeader() noexcept(false);
    void deserializeMatrix(Model &mesh) noexcept(false);
    VoxelArray readUncompressed(u32 sizeX, u32 sizeY, u32 sizeZ) noexcept(false);
    VoxelArray readCompressed(u32 sizeX, u32 sizeY, u32 sizeZ) noexcept(false);
};

/**
 * <p>
 *     A serializer for <b>Qubicle Binary (.qb)</b> files.
 * </p>
 * <p>
 *     No version restrictions exist.
 * </p>
 * Qubicle Geometry note:<ul>
 *     <li>y-axis points upwards</li>
 *     <li>z-axis 90 degrees to the right of x-axis</li>
 * </ul>
 */
class Serializer : public AbstractSerializer {
public:
    Serializer(OutputStream &ostream) : AbstractSerializer{ostream} {}

    ResultCode write(const Model &mesh) noexcept(false);

private:
    void serializeHeader(u32 modelSize) noexcept(false);
    void serializeMatrix(const Matrix &matrix) noexcept(false);
    void serializeUncompressed(const VoxelArray &array) noexcept(false);
};

}  // namespace qb
}  // namespace voxelio

#endif  // QUBICLE_BINARY_H
