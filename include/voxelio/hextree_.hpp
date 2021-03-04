#ifndef VXIO_HEXTREE_HPP
#define VXIO_HEXTREE_HPP
/*
 * hextree_.hpp
 * ------------
 * An internal header (not part of public API, do not include yourself).
 * Provides a hex tree data structure which is necessary for reducing palette sizes using k-means clustering.
 */

#include "bits.hpp"
#include "ileave.hpp"
#include "types.hpp"

#include <memory>

namespace voxelio {

namespace detail {

using hex_tree_value_type = u32;
using Vec4u8 = Vec<u8, 4>;

constexpr usize HEX_TREE_BRANCHING_FACTOR = 16;

struct HexTreeNodeBase {
    u16 childMask = 0;

    constexpr bool has(usize i) const
    {
        return getBit(childMask, i);
    }

    constexpr void add(usize i)
    {
        childMask = setBit(childMask, i);
    }
};

template <usize LEVEL>
struct HexTreeNode : public HexTreeNodeBase {
    static_assert(LEVEL != 0);

    using child_type = HexTreeNode<LEVEL - 1>;
    static constexpr usize level = LEVEL;

    std::unique_ptr<HexTreeNode<LEVEL - 1>> children[HEX_TREE_BRANCHING_FACTOR];
};

template <>
struct HexTreeNode<1> : public HexTreeNodeBase {
    using child_type = hex_tree_value_type;
    static constexpr usize level = 1;

    hex_tree_value_type values[HEX_TREE_BRANCHING_FACTOR];
};

constexpr u32 lengthSqr(Vec<int, 4> v)
{
    return static_cast<u32>(dot(v, v));
}

constexpr u32 distanceSqr(Vec4u8 p0, Vec4u8 p1)
{
    return lengthSqr(p0.cast<int>() - p1.cast<int>());
}

constexpr u32 ileave4b(u32 bytes)
{
    return static_cast<u32>(ileaveBytes_const<4>(bytes));
}

constexpr u32 dileave4b(u32 bytes)
{
    return static_cast<u32>(dileaveBytes_const<4>(bytes));
}

constexpr u32 pack4b(Vec4u8 v)
{
    return (u32{v[0]} << 24) | (u32{v[1]} << 16) | (u32{v[2]} << 8) | (u32{v[3]} << 0);
}

constexpr Vec4u8 unpack4b(u32 b)
{
    return Vec<u32, 4>{b >> 24, b >> 16, b >> 8, b >> 0}.cast<u8>();
}

}  // namespace detail

// HEX TREE ============================================================================================================

struct HexTree {
public:
    using value_type = detail::hex_tree_value_type;

    template <usize N>
    using Node = detail::HexTreeNode<N>;

private:
    using Vec4u8 = detail::Vec4u8;

    constexpr static usize DEPTH = 8;
    constexpr static usize BRANCHING_FACTOR = detail::HEX_TREE_BRANCHING_FACTOR;

    Node<DEPTH> root;

public:
    /**
     * @brief Inserts a color into the HexTree.
     * @param color the color
     * @param value the value of the color (palette index)
     */
    void insert(argb32 color, value_type value);

    value_type *find(argb32 color);

    const value_type *find(argb32 color) const;

    bool contains(argb32 color) const
    {
        return find(color) != nullptr;
    }

    template <typename F, std::enable_if_t<std::is_invocable_v<F, argb32, value_type>, int> = 0>
    void forEach(const F &action)
    {
        forEach_impl(0, root, std::move(action));
    }

    /**
     * @brief Finds the closest color and its value to a given color.
     * @param color the query color
     * @return the closest color and its value
     */
    std::pair<argb32, value_type> closest(argb32 color) const;

    /**
     * @brief Returns the distance of a given color to any point in the tree.
     * This is equivalent to the distance to the closest point.
     * @param color the query color
     * @return the distance to the closest point
     */
    u32 distanceSqr(argb32 color);

private:
    template <typename F, usize LEVEL>
    static void forEach_impl(u32 morton, Node<LEVEL> &node, const F &action)
    {
        for (u8 i = 0; i < BRANCHING_FACTOR; ++i) {
            if (not node.has(i)) {
                continue;
            }
            const u32 childMorton = (morton << 4) | i;
            if constexpr (LEVEL > 1) {
                forEach_impl(childMorton, *node.children[i], action);
            }
            else {
                u32 childPos = detail::dileave4b(childMorton);
                action(childPos, node.values[i]);
            }
        }
    }

    template <typename NodeType>
    static const value_type *find_impl(u32 morton, NodeType &node);

    template <usize LEVEL>
    static value_type *findOrCreate_impl(u32 morton, Node<LEVEL> &node, value_type paletteIndex);
};

}  // namespace voxelio

#endif  // HEXTREE_HPP
