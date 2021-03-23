#ifndef VXIO_VOXELARRAY_HPP
#define VXIO_VOXELARRAY_HPP

#include "color.hpp"
#include "types.hpp"

#include <memory>

namespace voxelio {
namespace detail {

template <typename TVoxelArray>
class VoxelArrayIterator {
public:
    using self_type = VoxelArrayIterator;
    using value_type = Voxel32;
    using reference = value_type &;
    using pointer = value_type *;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = usize;

    TVoxelArray *parent;
    usize index;

    VoxelArrayIterator(TVoxelArray &parent, usize index) : parent{&parent}, index{index}
    {
        VXIO_DEBUG_ASSERTM(index < parent.volume(), "index out of range");
        if (parent[index].isInvisible()) {
            ++(*this);
        }
    }

    // end constructor
    VoxelArrayIterator(TVoxelArray &parent) : parent{&parent}, index{parent.volume()} {}

    void operator++()
    {
        const usize lim = parent->volume();
        while (++index < lim) {
            if ((*parent)[index].isVisible()) break;
        }
    }

    bool operator==(const self_type &b)
    {
        return this->index == b.index;
    }

    bool operator!=(const self_type &b)
    {
        return this->index != b.index;
    }

    value_type operator*()
    {
        Vec3size pos = parent->posOf(index);
        return Voxel32{pos.cast<i32>(), parent->voxels[index]};
    }
};

}  // namespace detail

/**
 * @brief An 3D, dynamically sized array.
 * This class is optimized for randomly accessing, setting, drawing etc. of voxels.
 */
class VoxelArray {
private:
    using bits8_t = uint_least8_t;

public:
    using iterator = detail::VoxelArrayIterator<VoxelArray>;
    using const_iterator = detail::VoxelArrayIterator<const VoxelArray>;

    friend iterator;
    friend const_iterator;

private:
    Vec3size size;
    usize _sizeXY, _volume;
    std::unique_ptr<Color32[]> voxels;

    static std::unique_ptr<Color32[]> copyContents(Color32 source[], usize size);

    VoxelArray(std::unique_ptr<Color32[]> voxels, usize x, usize y, usize z);

public:
    VoxelArray(usize x, usize y, usize z);

    explicit VoxelArray(Vec3size size) : VoxelArray{size.x(), size.y(), size.z()} {}
    VoxelArray() : VoxelArray{nullptr, 0, 0, 0} {}

    VoxelArray(const VoxelArray &copyOf)
        : VoxelArray{
              copyContents(copyOf.voxels.get(), copyOf._volume), copyOf.size.x(), copyOf.size.y(), copyOf.size.z()}
    {
    }

    VoxelArray(VoxelArray &&) = default;

private:
    usize indexOf(Vec3size pos) const
    {
        VXIO_DEBUG_ASSERT_LT(pos.x(), size.x());
        VXIO_DEBUG_ASSERT_LT(pos.y(), size.y());
        VXIO_DEBUG_ASSERT_LT(pos.z(), size.z());
        return (pos.z() * _sizeXY) + (pos.y() * size.x()) + pos.x();
    }

    Vec3size posOf(usize index) const
    {
        return {index % size.x(), index / size.x() % size.y(), index / _sizeXY};
    }

public:
    Vec3size dimensions() const
    {
        return size;
    }

    usize volume() const
    {
        return _volume;
    }

    usize countVoxels() const;

    const Color32 &at(Vec3size pos) const
    {
        VXIO_DEBUG_ASSERT_LT(pos.x(), size.x());
        VXIO_DEBUG_ASSERT_LT(pos.y(), size.y());
        VXIO_DEBUG_ASSERT_LT(pos.z(), size.z());
        return this->voxels[indexOf(pos)];
    }

    Color32 &at(Vec3size pos)
    {
        VXIO_DEBUG_ASSERT_LT(pos.x(), size.x());
        VXIO_DEBUG_ASSERT_LT(pos.y(), size.y());
        VXIO_DEBUG_ASSERT_LT(pos.z(), size.z());
        return this->voxels[indexOf(pos)];
    }

    /**
     * <p>
     * Returns whether the array contains a voxel at the given position.
     * </p>
     * <p>
     * This is the case unless the voxel array contains a completely transparent voxel <code>(alpha = 0) </code>
     * at the coordinates.
     * </p>
     *
     * @param x the x-coordinate
     * @param y the y-coordinate
     * @param z the z-coordinate
     * @return whether the array contains a voxel
     */
    bool contains(Vec3size pos) const
    {
        return at(pos).isVisible();
    }

    // SETTERS

    void remove(Vec3size pos)
    {
        at(pos) = ArgbColor::INVISIBLE_WHITE;
    }

    /**
     * Fills the entire voxel array with a single rgb value.
     *
     * @param rgb the rgb value
     */
    void fill(Color32 color);

    /**
     * Clears the voxel array.
     */
    void clear();

    /**
     * Fills this voxel array with another array at a given offset.
     *
     * @param array the array to paste this one with
     */
    void paste(const VoxelArray &array, usize x, usize y, usize z);

    // MISC

    std::string toString() const;

    explicit operator std::string() const
    {
        return toString();
    }

    // OPERATORS

    VoxelArray &operator=(VoxelArray &&) = default;

    const Color32 &operator[](const usize index) const
    {
        VXIO_DEBUG_ASSERT(index < _volume);
        return voxels[index];
    }

    Color32 &operator[](const usize index)
    {
        VXIO_DEBUG_ASSERT(index < _volume);
        return voxels[index];
    }

    const Color32 &operator[](Vec3size pos) const
    {
        return at(pos);
    }

    Color32 &operator[](Vec3size pos)
    {
        return at(pos);
    }

    /**
     * Returns whether this array is equal to another array. This condition is met of the arrays are equal in size and
     * equal in content.
     *
     * @param array the array
     * @return whether the arrays are equal
     */
    bool operator==(const VoxelArray &other) const;

    /**
     * Returns whether this array is not equal to another array. This condition is met of the arrays are equal in size
     * and equal in content.
     *
     * @param array the array
     * @return whether the arrays are equal
     */
    bool operator!=(const VoxelArray &other) const
    {
        return not(*this == other);
    }

    // ITERATIONS

    template <typename F, std::enable_if_t<std::is_invocable_v<F, Vec3size>, int> = 0>
    void forEachPosition(const F &action) const
    {
        for (usize z = 0; z < size.z(); z++)
            for (usize y = 0; y < size.y(); y++)
                for (usize x = 0; x < size.x(); x++)
                    action(Vec3size{x, y, z});
    }

    iterator begin()
    {
        return {*this, 0};
    }
    iterator end()
    {
        return {*this};
    }

    const_iterator begin() const
    {
        return {*this, 0};
    }
    const_iterator end() const
    {
        return {*this};
    }
};

}  // namespace voxelio

#endif  // VOXELARRAY_HPP
