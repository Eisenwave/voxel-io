#ifndef VXIO_SVX_HPP
#define VXIO_SVX_HPP

#include "../types.hpp"
#include "../voxelarray.hpp"
#include "../voxelio.hpp"

#include "../3rd_party/miniz_cpp/miniz_cppfwd.hpp"

#include <vector>

namespace voxelio::svx {

enum class ChannelType {
    /** Represents an approximation to the percent of voxel fill. */
    DENSITY,
    /** COLOR – RGB. */
    COLOR_RGB,
    /** Fraction of amount a given material used in a voxel (all densities must sum to 100% or 0%). */
    MATERIAL,
    /** Custom value without defined semantic meaning. */
    CUSTOM
};

struct Channel {
    /** The type of information stored in this channel. */
    ChannelType type;
    /** The number of bits used for the channel, default is 8 bits. */
    size_t bits = 0;
    /** Optional: The material or custom id of this channel. Not necessary for density or color. */
    size_t id = 0;
    /** The printf-pattern for the paths of slices belonging to this channel. */
    std::string fileNameFormat = "%04d.png";

    std::string fullFileNameFormat() const;
    std::string toXml() const;
};

/** A struct representing the manifest of an SVX file. */
class Manifest {
public:
    /** The specification version this manifest follows. */
    static constexpr const char *VERSION = "1.0";

private:
    /** The number of voxels in each direction. */
    Vec3size gridSize_;
    /** The origin of the grid in physical space in meters. */
    Vec3f origin_ = {0, 0, 0};
    /** The axis(X,Y,Z) orthogonal to the slices plane. */
    Axis slicesOrientation_;
    /** Number of bits used per voxel for density (1-16). */
    uint32_t subvoxelBits_;
    /** The physical size of each voxel in meters. */
    float voxelSize_;

public:
    std::vector<Channel> channels{};

    Manifest(Vec3size gridSize,
             Vec3f origin = {0, 0, 0},
             Axis slicesOrientation = Axis::Y,
             uint32_t subvoxelBits = 1,
             float voxelSize = 1 / 16.f)
        : gridSize_{std::move(gridSize)}
        , origin_{origin}
        , slicesOrientation_{slicesOrientation}
        , subvoxelBits_{subvoxelBits}
        , voxelSize_{voxelSize}
    {
        VXIO_ASSERT_NE(this->gridSize_.x(), 0);
        VXIO_ASSERT_NE(this->gridSize_.y(), 0);
        VXIO_ASSERT_NE(this->gridSize_.z(), 0);
        VXIO_ASSERT(subvoxelBits > 0 && subvoxelBits <= 16);
    }
    Manifest(Manifest &&) = default;
    Manifest(const Manifest &) = default;
    Manifest &operator=(Manifest &&) = default;
    Manifest &operator=(const Manifest &) = default;

    constexpr Vec3size gridSize() const
    {
        return gridSize_;
    }

    constexpr Vec3f origin() const
    {
        return origin_;
    }

    constexpr Axis slicesOrientation() const
    {
        return slicesOrientation_;
    }

    constexpr uint32_t subvoxelBits() const
    {
        return subvoxelBits_;
    }

    constexpr float voxelSize() const
    {
        return voxelSize_;
    }

    bool matches(const VoxelArray &voxels) const;

    std::string toXml() const;
};

struct SimpleVoxels {
    Manifest manifest;
    VoxelArray voxels;

    SimpleVoxels(Manifest manifest, VoxelArray voxels) : manifest{std::move(manifest)}, voxels{std::move(voxels)}
    {
        VXIO_ASSERT(this->manifest.matches(this->voxels));
    }
    SimpleVoxels(SimpleVoxels &&) = default;
    SimpleVoxels(const SimpleVoxels &) = default;
    // SimpleVoxels& operator=(SimpleVoxels&&)=default;
    // SimpleVoxels& operator=(const SimpleVoxels&)=default;
};

class Serializer : public AbstractSerializer {
public:
    Serializer(OutputStream &ostream) : AbstractSerializer{ostream} {}

    ResultCode write(const SimpleVoxels &svx) noexcept(false);

private:
    template <Axis AXIS>
    ResultCode writeChannel(miniz_cpp::zip_file &archive,
                            const SimpleVoxels &svx,
                            const Channel &channel) noexcept(false);
};

}  // namespace voxelio::svx

#endif  // SVX_HPP
