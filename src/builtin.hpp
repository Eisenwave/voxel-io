#ifndef VXIO_BUILTIN_HPP
#define VXIO_BUILTIN_HPP

#include "build.hpp"

#include <cstdint>

#ifdef VXIO_GNU_OR_CLANG

// VXIO_HAS_BUILTIN(builtin):
//     Checks whether a builtin exists.
//     This only works on recent versions of clang, as this macro does not exist in gcc.
//     For gcc, the result is always 1 (true), optimistically assuming that the builtin exists.
#ifdef __has_builtin
#define VXIO_HAS_BUILTIN_HAS_BUILTIN
#define VXIO_HAS_BUILTIN(builtin) __has_builtin(builtin)
#else
#define VXIO_HAS_BUILTIN(builtin) 1
#endif

// VXIO_UNREACHABLE():
//     Signals to the compiler that a given code point is unreachable.
//     This macro is always defined, but does nothing if no builtin is available.
#if VXIO_HAS_BUILTIN(__builtin_unreachable)
#define VXIO_HAS_BUILTIN_UNREACHABLE
#define VXIO_UNREACHABLE() __builtin_unreachable()
#else
#define VXIO_UNREACHABLE()
#endif

namespace voxelio::builtin {

// void trap():
//     Executes an abnormal instruction or otherwise exits the program immediately.
//     This bypasses even std::terminate() and is completely unhandled.
//     This function always exists and will call std::terminate() by default.
#if VXIO_HAS_BUILTIN(__builtin_trap)
#define VXIO_HAS_BUILTIN_TRAP
[[noreturn]] inline void trap()
{
    __builtin_trap();
}
#else
[[noreturn]] void trap();
#endif

// bool isConstantEvaluated():
//     To be used in an if-statement to verify whether the current context is constant-evaluated.
//     This builtin can potentially not exist and it has no sane default.
//     clang 9+ and gcc 9+ support this builtin, even for C++17, allowing for implementation of
//     std::is_constant_evaluated() before C++20.
#if VXIO_HAS_BUILTIN(__builtin_is_constant_evaluated)
#define VXIO_HAS_BUILTIN_IS_CONSTANT_EVALUATED
constexpr bool isConstantEvaluated()
{
    return __builtin_is_constant_evaluated();
}
#endif

// int countLeadingZeros(unsigned ...):
//     Counts the number of leading zeros in an unsigned integer type.
//     The result of countLeadingZeros(0) is undefined for all types.
#if VXIO_HAS_BUILTIN(__builtin_clz) && VXIO_HAS_BUILTIN(__builtin_clzl) && VXIO_HAS_BUILTIN(__builtin_clzll)
#define VXIO_HAS_BUILTIN_CLZ
constexpr int countLeadingZeros(unsigned char x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned char)) * 8};
}

constexpr int countLeadingZeros(unsigned short x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned short)) * 8};
}

constexpr int countLeadingZeros(unsigned int x) noexcept
{
    return __builtin_clz(x);
}

constexpr int countLeadingZeros(unsigned long x) noexcept
{
    return __builtin_clzl(x);
}

constexpr int countLeadingZeros(unsigned long long x) noexcept
{
    return __builtin_clzll(x);
}
#endif

// uintXX_t byteSwap(uintXX_t ...):
//     Swaps the bytes of any uintXX type.
//     This reverses the byte order (little-endian/big-endian).
//     This does nothing for byteSwap(uint8_t).
#if VXIO_HAS_BUILTIN(__builtin_bswap16) && VXIO_HAS_BUILTIN(__builtin_bswap32) && VXIO_HAS_BUILTIN(__builtin_bswap64)
#define VXIO_HAS_BUILTIN_BSWAP
constexpr uint8_t byteSwap(uint8_t x) noexcept
{
    return x;
}

constexpr uint16_t byteSwap(uint16_t x) noexcept
{
    return __builtin_bswap16(x);
}

constexpr uint32_t byteSwap(uint32_t x) noexcept
{
    return __builtin_bswap32(x);
}

constexpr uint64_t byteSwap(uint64_t x) noexcept
{
    return __builtin_bswap64(x);
}
#endif

#else

#define VXIO_UNREACHABLE()
#define VXIO_TRAP()

#endif

}  // namespace voxelio::builtin

#endif  // BUILTIN_HPP
