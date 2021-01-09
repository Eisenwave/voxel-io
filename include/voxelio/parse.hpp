#ifndef VXIO_PARSE_HPP
#define VXIO_PARSE_HPP

#include "sstreamwrap.hpp"

#include <charconv>
#include <iosfwd>
#include <string>
#include <type_traits>

namespace voxelio {

namespace detail {

template <typename T>
bool parseUsingStream(const std::string &str, T &out) noexcept
{
    std::stringstream *stream = detail::stringstream_make(str);
    *stream >> out;
    bool result = not detail::stringstream_fail(stream);
    detail::stringstream_free(stream);
    return result;
}

}  // namespace detail

template <typename Int, std::enable_if_t<not std::is_same_v<Int, bool>, int> = 0>
bool parseInt(const std::string &str, Int &out, unsigned base = 10) noexcept
{
    const auto *begin = str.data();
    const auto *end = str.data() + str.size();
    return std::from_chars(begin, end, out, base).ec == std::errc{};
}

template <typename Float, std::enable_if_t<std::is_floating_point_v<Float>, int> = 0>
bool parseFloat(const std::string &str, Float &out) noexcept
{
    try {
        if constexpr (std::is_same_v<Float, float>) {
            out = std::stof(str);
        }
        else if constexpr (std::is_same_v<Float, double>) {
            out = std::stod(str);
        }
        else if constexpr (std::is_same_v<Float, long double>) {
            out = std::stold(str);
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

template <typename T>
bool parse(const std::string &str, T &out) noexcept
{
    if constexpr (std::is_integral_v<T>) {
        return parseInt(str, out);
    }
    else if constexpr (std::is_floating_point_v<T>) {
        return parseFloat(str, out);
    }
    else {
        return detail::parseUsingStream(str, out);
    }
}

template <typename T>
bool parseHex(const std::string &str, T &out) noexcept
{
    return parseInt(str, out, 16);
}

template <typename T>
bool parseDec(const std::string &str, T &out) noexcept
{
    return parseInt(str, out, 10);
}

template <typename T>
bool parseOct(const std::string &str, T &out) noexcept
{
    return parseInt(str, out, 8);
}

template <typename T>
bool parseBin(const std::string &str, T &out) noexcept
{
    return parseInt(str, out, 2);
}

}  // namespace voxelio

#endif  // PARSE_HPP
