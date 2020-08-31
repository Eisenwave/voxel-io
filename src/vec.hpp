#ifndef VXIO_VEC_HPP
#define VXIO_VEC_HPP

#include "assert.hpp"
#include "primitives.hpp"

#include <cstddef>
#include <string>
#include <type_traits>

namespace voxelio {

template <typename T, usize N>
class Vec {
public:
    using value_t = T;
    using magnitude_t = std::conditional_t<std::is_floating_point_v<T>, T, double>;

    constexpr static usize size = N;

    constexpr static Vec filledWith(const T &t);

    constexpr static Vec zero()
    {
        return Vec::filledWith(0);
    }

    constexpr static Vec one()
    {
        return Vec::filledWith(1);
    }

private:
    T content[N];

public:
    constexpr Vec() = default;
    constexpr Vec(const Vec &) = default;
    constexpr Vec(Vec &&) = default;

    constexpr explicit Vec(const T data[N]) : content{}
    {
        std::copy(data, data + N, begin());
    }

    template <typename... Args,
              std::enable_if_t<(((std::is_same_v<T, Args> && ...) ||
                                 std::is_same_v<T, std::common_type_t<T, Args...>>) &&sizeof...(Args) <= N),
                               int> = 0>
    constexpr Vec(Args... args) : content{static_cast<T>(std::move(args))...}
    {
    }

    template <typename To>
    constexpr auto cast() const;

    constexpr T &x()
    {
        return this->operator[](0);
    }

    constexpr T &y()
    {
        return this->operator[](1);
    }

    constexpr T &z()
    {
        return this->operator[](2);
    }

    constexpr const T &x() const
    {
        return this->operator[](0);
    }

    constexpr const T &y() const
    {
        return this->operator[](1);
    }

    constexpr const T &z() const
    {
        return this->operator[](2);
    }

    constexpr T *data()
    {
        return content;
    }

    constexpr const T *data() const
    {
        return content;
    }

    template <usize i>
    constexpr T get()
    {
        return this->operator[](i);
    }

    // ITERATION

    T *begin()
    {
        return content;
    }

    T *end()
    {
        return content + N;
    }

    const T *begin() const
    {
        return content;
    }

    const T *end() const
    {
        return content + N;
    }

    // MISC

    std::string toString() const;

    // OPERATORS

    constexpr Vec &operator=(const Vec &) = default;
    constexpr Vec &operator=(Vec &&) = default;

    constexpr Vec &operator+=(const Vec &other);
    constexpr Vec &operator-=(const Vec &other);
    constexpr Vec &operator*=(T scalar);
    constexpr Vec &operator/=(T scalar);

    constexpr T &operator[](usize index)
    {
        VXIO_DEBUG_ASSERT_LE(index, N);
        return content[index];
    }

    constexpr const T &operator[](usize index) const
    {
        VXIO_DEBUG_ASSERT_LE(index, N);
        return content[index];
    }
};

template <typename T, usize N>
constexpr Vec<T, N> Vec<T, N>::filledWith(const T &t)
{
    Vec<T, N> result{};
    for (usize i = 0; i < N; i++)
        result[i] = t;
    return result;
}

template <typename T, usize N>
std::string Vec<T, N>::toString() const
{
    // We could also use stringstream here, but that would require an iostream include EVERYWHERE
    std::string stream = "Vec{";
    stream.reserve(16 * N);

    auto iter = this->begin();
    auto end = this->end();
    if (iter != end) {
        stream += stringify(*iter);
        ++iter;
    }
    while (iter != end) {
        stream += ", ";
        stream += stringify(*iter);
        ++iter;
    }
    return stream + '}';
}

// CASTING

namespace detail {

template <typename T>
struct is_vec : std::false_type {
};

template <typename T, usize N>
struct is_vec<Vec<T, N>> : std::true_type {
};

}  // namespace detail

template <typename T>
constexpr bool isVec = detail::is_vec<T>::value;

template <typename T, usize N>
template <typename To>
constexpr auto Vec<T, N>::cast() const
{
    if constexpr (std::is_same_v<To, value_t>) {
        return *this;
    }
    else if constexpr (isVec<To>) {
        return cast<typename To::value_t>(*this);
    }
    else {
        Vec<To, N> result{};
        for (usize i = 0; i < N; ++i)
            result[i] = static_cast<To>(operator[](i));
        return result;
    }
}

// COMPARISON

template <typename L, typename R, usize N>
constexpr bool operator==(const Vec<L, N> &a, const Vec<R, N> &b)
{
    for (usize i = 0; i < N; i++)
        if (a[i] != b[i]) return false;
    return true;
}

template <typename L, typename R, usize N>
constexpr bool operator!=(const Vec<L, N> &a, const Vec<R, N> &b)
{
    for (usize i = 0; i < N; i++)
        if (a[i] != b[i]) return true;
    return false;
}

// ARITHMETIC

// addition
template <typename L, typename R, usize N>
constexpr Vec<std::common_type_t<L, R>, N> operator+(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result{};
    for (usize i = 0; i < N; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

// subtraction
template <typename L, typename R, usize N>
constexpr Vec<std::common_type_t<L, R>, N> operator-(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result{};
    for (usize i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

// scalar product
template <typename T, typename S, usize N>
constexpr Vec<std::common_type_t<T, S>, N> operator*(const Vec<T, N> &a, const S &t)
{
    Vec<std::common_type_t<T, S>, N> result{};
    for (usize i = 0; i < N; i++) {
        result[i] = a[i] * t;
    }
    return result;
}

// inverse scalar product
template <typename T, typename S, usize N>
constexpr Vec<std::common_type_t<T, S>, N> operator/(const Vec<T, N> &a, const S &t)
{
    Vec<std::common_type_t<T, S>, N> result{};
    for (usize i = 0; i < N; i++) {
        result[i] = a[i] / t;
    }
    return result;
}

// dot product
template <typename L, typename R, usize N>
constexpr std::common_type_t<L, R> dot(const Vec<L, N> &a, const Vec<R, N> &b)
{
    std::common_type_t<L, R> result = 0;
    for (usize i = 0; i < N; i++) {
        result += a[i] * b[i];
    }
    return result;
}

// dot product
template <typename L, typename R, usize N>
constexpr Vec<std::common_type_t<L, R>, N> mul(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result;
    for (usize i = 0; i < N; i++) {
        result[i] = a[i] * b[i];
    }
    return result;
}

// addition
template <typename T, usize N>
constexpr Vec<T, N> &Vec<T, N>::operator+=(const Vec<T, N> &a)
{
    for (usize i = 0; i < N; i++) {
        (*this)[i] += a[i];
    }
    return *this;
}

// subtraction
template <typename T, usize N>
constexpr Vec<T, N> &Vec<T, N>::operator-=(const Vec<T, N> &a)
{
    for (usize i = 0; i < N; i++) {
        (*this)[i] += a[i];
    }
    return *this;
}

// subtraction
template <typename T, usize N>
constexpr Vec<T, N> &Vec<T, N>::operator*=(T s)
{
    for (usize i = 0; i < N; i++) {
        (*this)[i] *= s;
    }
    return *this;
}

// division
template <typename T, usize N>
constexpr Vec<T, N> &Vec<T, N>::operator/=(T s)
{
    for (usize i = 0; i < N; i++) {
        (*this)[i] /= s;
    }
    return *this;
}

}  // namespace voxelio

namespace std {
template <typename T, std::size_t N>
struct hash<voxelio::Vec<T, N>> {
    constexpr std::size_t operator()(const voxelio::Vec<T, N> &s) const noexcept
    {
        std::size_t result = 1;
        for (std::size_t i = 0; i < N; ++i) {
            std::size_t h = std::hash<T>{}(s[i]);
            h ^= h >> 32;
            result = 31 * result + h;
        }
        return result;
    }
};

}  // namespace std

namespace voxelio {

using Vec2c = Vec<char, 2>;
using Vec2s = Vec<short, 2>;
using Vec2i = Vec<int, 2>;
using Vec2l = Vec<long, 2>;
using Vec2ll = Vec<long long, 2>;
using Vec2uc = Vec<unsigned char, 2>;
using Vec2us = Vec<unsigned short, 2>;
using Vec2ui = Vec<unsigned int, 2>;
using Vec2u = Vec<unsigned, 2>;
using Vec2ul = Vec<unsigned long, 2>;
using Vec2ull = Vec<unsigned long long, 2>;
using Vec2b = Vec<bool, 2>;
using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;
using Vec2ld = Vec<long double, 2>;

using Vec2i8 = Vec<i8, 2>;
using Vec2i16 = Vec<i16, 2>;
using Vec2i32 = Vec<i32, 2>;
using Vec2i64 = Vec<i64, 2>;
using Vec2imax = Vec<imax, 2>;
using Vec2u8 = Vec<u8, 2>;
using Vec2u16 = Vec<u16, 2>;
using Vec2u32 = Vec<u32, 2>;
using Vec2u64 = Vec<u64, 2>;
using Vec2umax = Vec<umax, 2>;
using Vec2size = Vec<usize, 2>;

using Vec3c = Vec<char, 3>;
using Vec3s = Vec<short, 3>;
using Vec3i = Vec<int, 3>;
using Vec3l = Vec<long, 3>;
using Vec3ll = Vec<long long, 3>;
using Vec3uc = Vec<unsigned char, 3>;
using Vec3us = Vec<unsigned short, 3>;
using Vec3ui = Vec<unsigned int, 3>;
using Vec3u = Vec<unsigned, 3>;
using Vec3ul = Vec<unsigned long, 3>;
using Vec3ull = Vec<unsigned long long, 3>;
using Vec3b = Vec<bool, 3>;
using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;
using Vec3ld = Vec<long double, 3>;

using Vec3i8 = Vec<i8, 3>;
using Vec3i16 = Vec<i16, 3>;
using Vec3i32 = Vec<i32, 3>;
using Vec3i64 = Vec<i64, 3>;
using Vec3imax = Vec<imax, 3>;
using Vec3u8 = Vec<u8, 3>;
using Vec3u16 = Vec<u16, 3>;
using Vec3u32 = Vec<u32, 3>;
using Vec3u64 = Vec<u64, 3>;
using Vec3umax = Vec<umax, 3>;
using Vec3size = Vec<usize, 3>;

}  // namespace voxelio

#endif  // VXIO_VEC_HPP
