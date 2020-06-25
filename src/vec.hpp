#ifndef VXIO_VEC_HPP
#define VXIO_VEC_HPP

#include "assert.hpp"

#include <cstddef>
#include <string>
#include <type_traits>

namespace voxelio {

template <typename T, size_t N>
class Vec {
public:
    using value_t = T;
    using magnitude_t = std::conditional_t<std::is_floating_point_v<T>, T, double>;

    constexpr static size_t size = N;

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

    template <size_t i>
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

    constexpr T &operator[](size_t index)
    {
        VXIO_DEBUG_ASSERT_LE(index, N);
        return content[index];
    }

    constexpr const T &operator[](size_t index) const
    {
        VXIO_DEBUG_ASSERT_LE(index, N);
        return content[index];
    }
};

template <typename T, size_t N>
constexpr Vec<T, N> Vec<T, N>::filledWith(const T &t)
{
    Vec<T, N> result{};
    for (size_t i = 0; i < N; i++)
        result[i] = t;
    return result;
}

template <typename T, size_t N>
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

template <typename T>
struct is_vec : std::false_type {
};

template <typename T, size_t N>
struct is_vec<Vec<T, N>> : std::true_type {
};

template <typename T>
constexpr bool is_vec_v = is_vec<T>::value;

template <typename To, typename Source, size_t N>
constexpr auto static_vec_cast(const Vec<Source, N> &source)
{
    if constexpr (std::is_same_v<To, Source>) {
        return source;
    }
    else if constexpr (is_vec_v<To>) {
        return static_vec_cast<typename To::value_t, Source, N>(source);
    }
    else {
        Vec<To, N> result{};
        for (size_t i = 0; i < N; ++i)
            result[i] = static_cast<To>(source[i]);
        return result;
    }
}

// COMPARISON

template <typename L, typename R, size_t N>
constexpr bool operator==(const Vec<L, N> &a, const Vec<R, N> &b)
{
    for (size_t i = 0; i < N; i++)
        if (a[i] != b[i]) return false;
    return true;
}

template <typename L, typename R, size_t N>
constexpr bool operator!=(const Vec<L, N> &a, const Vec<R, N> &b)
{
    for (size_t i = 0; i < N; i++)
        if (a[i] != b[i]) return true;
    return false;
}

// ARITHMETIC

// addition
template <typename L, typename R, size_t N>
constexpr Vec<std::common_type_t<L, R>, N> operator+(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

// subtraction
template <typename L, typename R, size_t N>
constexpr Vec<std::common_type_t<L, R>, N> operator-(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

// scalar product
template <typename T, typename S, size_t N>
constexpr Vec<std::common_type_t<T, S>, N> operator*(const Vec<T, N> &a, const S &t)
{
    Vec<std::common_type_t<T, S>, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = a[i] * t;
    }
    return result;
}

// inverse scalar product
template <typename T, typename S, size_t N>
constexpr Vec<std::common_type_t<T, S>, N> operator/(const Vec<T, N> &a, const S &t)
{
    Vec<std::common_type_t<T, S>, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = a[i] / t;
    }
    return result;
}

// dot product
template <typename L, typename R, size_t N>
constexpr std::common_type_t<L, R> dot(const Vec<L, N> &a, const Vec<R, N> &b)
{
    std::common_type_t<L, R> result = 0;
    for (size_t i = 0; i < N; i++) {
        result += a[i] * b[i];
    }
    return result;
}

// dot product
template <typename L, typename R, size_t N>
constexpr Vec<std::common_type_t<L, R>, N> mul(const Vec<L, N> &a, const Vec<R, N> &b)
{
    Vec<std::common_type_t<L, R>, N> result;
    for (size_t i = 0; i < N; i++) {
        result[i] = a[i] * b[i];
    }
    return result;
}

// addition
template <typename T, size_t N>
constexpr Vec<T, N> &Vec<T, N>::operator+=(const Vec<T, N> &a)
{
    for (size_t i = 0; i < N; i++) {
        (*this)[i] += a[i];
    }
    return *this;
}

// subtraction
template <typename T, size_t N>
constexpr Vec<T, N> &Vec<T, N>::operator-=(const Vec<T, N> &a)
{
    for (size_t i = 0; i < N; i++) {
        (*this)[i] += a[i];
    }
    return *this;
}

// subtraction
template <typename T, size_t N>
constexpr Vec<T, N> &Vec<T, N>::operator*=(T s)
{
    for (size_t i = 0; i < N; i++) {
        (*this)[i] *= s;
    }
    return *this;
}

// division
template <typename T, size_t N>
constexpr Vec<T, N> &Vec<T, N>::operator/=(T s)
{
    for (size_t i = 0; i < N; i++) {
        (*this)[i] /= s;
    }
    return *this;
}

}  // namespace voxelio

namespace std {
template <typename T, size_t N>
struct hash<voxelio::Vec<T, N>> {
    constexpr std::size_t operator()(const voxelio::Vec<T, N> &s) const noexcept
    {
        std::size_t result = 1;
        for (size_t i = 0; i < N; ++i) {
            std::size_t h = std::hash<T>{}(s[i]);
            h ^= h >> 32;
            result = 31 * result + h;
        }
        return result;
    }
};
}  // namespace std

#endif  // VXIO_VEC_HPP
