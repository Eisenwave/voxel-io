#ifndef VXIO_CSTRINGVIEW_HPP
#define VXIO_CSTRINGVIEW_HPP

#include "assert.hpp"
#include "builtin.hpp"
#include "util.hpp"

#include <string>
#include <string_view>

namespace voxelio {

/**
 * @brief A non-owning, sized sequence of null-terminated characters. Similar to std::string_view, but has
 * null-termination guarantee.
 *
 * This class satisfies the Container requirement.
 */
class CStringView final {
public:
    using value = char;
    using reference = value &;
    using const_reference = const value &;
    using pointer = value *;
    using const_pointer = const value *;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    const char *str;
    std::size_t len;

public:
    constexpr CStringView() noexcept : str{""}, len{0} {}

    CStringView(const std::string &str) noexcept : str{str.c_str()}, len{str.length()} {}

    constexpr CStringView(const char *str) noexcept : str{str}, len{std::char_traits<char>::length(str)}
    {
        VXIO_DEBUG_ASSERT_NOTNULL(str);
    }

    constexpr CStringView(std::nullptr_t) = delete;

    constexpr size_type size() const noexcept
    {
        return len;
    }

    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }

    constexpr size_type max_size() const noexcept
    {
        return ~size_type{0};
    }

    constexpr size_type length() const noexcept
    {
        return len;
    }

    constexpr const_pointer data() const noexcept
    {
        VXIO_DEBUG_ASSERT_NOTNULL(str);
        VXIO_ASSUME(str != nullptr);
        return str;
    }

    constexpr const_pointer c_str() const noexcept
    {
        return data();
    }

    constexpr const_pointer begin() const noexcept
    {
        return str;
    }

    constexpr const_pointer end() const noexcept
    {
        return str + len;
    }

    constexpr const_pointer cbegin() const noexcept
    {
        return str;
    }

    constexpr const_pointer cend() const noexcept
    {
        return str + len;
    }

    constexpr int compare(CStringView other) const noexcept
    {
        return std::char_traits<char>::compare(this->data(), other.data(), len);
    }

    constexpr void swap(CStringView &other) noexcept
    {
        trivialSwap(this->len, other.len);
        trivialSwap(this->str, other.str);
    }

    constexpr operator const_pointer() const noexcept
    {
        return data();
    }

    constexpr operator std::string_view() const noexcept
    {
        return {data(), size()};
    }

    friend constexpr bool operator==(CStringView a, CStringView b) noexcept
    {
        return a.size() == b.size() && a.compare(b) == 0;
    }

    friend constexpr bool operator!=(CStringView a, CStringView b) noexcept
    {
        return not(a == b);
    }
};

constexpr void swap(CStringView &a, CStringView &b) noexcept
{
    a.swap(b);
}

}  // namespace voxelio

#endif  // VXIO_CSTRINGVIEW_HPP
