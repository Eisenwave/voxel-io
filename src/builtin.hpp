#ifndef VXIO_BUILTIN_HPP
#define VXIO_BUILTIN_HPP
/*
 * builtin.hpp
 * -----------
 * Forwards to builtin functions in a compiler-independent way.
 */

#include "build.hpp"

#include <cstdint>

// COMPILER AGNOSTIC ===================================================================================================

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

// GCC / CLANG BUILTINS ================================================================================================

// VXIO_UNREACHABLE():
//     Signals to the compiler that a given code point is unreachable.
//     This macro is always defined, but does nothing if no builtin is available.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_unreachable)
#define VXIO_HAS_BUILTIN_UNREACHABLE
#define VXIO_UNREACHABLE() __builtin_unreachable()
#else
#define VXIO_UNREACHABLE()
#endif

// VXIO_ASSUME():
//     Signals to the compiler that a given expression always evaluates to true.
//     This macro is always defined, but does nothing if no builtin is available.
//     Note that the expression inside of VXIO_ASSUME() is evaluated, although the result is unused.
//     This makes it unsafe to use when the expression has side effects.
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_assume)
#define VXIO_HAS_BUILTIN_ASSUME
#define VXIO_ASSUME(condition) __builtin_assume(condition)

#elif defined(VXIO_GNU)
#define VXIO_HAS_BUILTIN_ASSUME
#define VXIO_ASSUME(condition) static_cast<bool>(condition) ? void(0) : VXIO_UNREACHABLE()

#elif defined(VXIO_MSVC)
#define VXIO_HAS_BUILTIN_ASSUME
#define VXIO_ASSUME(condition) __assume(condition)

#else
#define VXIO_ASSUME(condition)
#endif

namespace voxelio::builtin {

// void trap():
//     Executes an abnormal instruction or otherwise exits the program immediately.
//     This bypasses even std::terminate() and is completely unhandled.
//     This function always exists and will call std::terminate() by default.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_trap)
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
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_is_constant_evaluated) || defined(VXIO_GNU) && VXIO_GNU >= 9
#define VXIO_HAS_BUILTIN_IS_CONSTANT_EVALUATED
constexpr bool isConstantEvaluated()
{
    return __builtin_is_constant_evaluated();
}
#endif

// int countLeadingRedundantSignBits(unsigned ...):
//     Counts the number of redundant sign bits, i.o.w. the number of bits following the sign bit which are equal to it.
//     This is the number of leading zeros minus one for positive numbers.
//     For negative numbers, it is the number of leading ones - 1.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_clrsb) && VXIO_HAS_BUILTIN(__builtin_clrsbl) && \
    VXIO_HAS_BUILTIN(__builtin_clrsbll)
#define VXIO_HAS_BUILTIN_CLRSB
constexpr int countLeadingRedundantSignBits(char x) noexcept
{
    return __builtin_clrsb(x);
}

constexpr int countLeadingRedundantSignBits(short x) noexcept
{
    return __builtin_clrsb(x);
}

constexpr int countLeadingRedundantSignBits(int x) noexcept
{
    return __builtin_clrsb(x);
}

constexpr int countLeadingRedundantSignBits(long x) noexcept
{
    return __builtin_clrsbl(x);
}

constexpr int countLeadingRedundantSignBits(long long x) noexcept
{
    return __builtin_clrsbll(x);
}
#endif

// int countLeadingZeros(unsigned ...):
//     Counts the number of leading zeros in an unsigned integer type.
//     The result of countLeadingZeros(0) is undefined for all types.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_clz) && VXIO_HAS_BUILTIN(__builtin_clzl) && \
    VXIO_HAS_BUILTIN(__builtin_clzll)
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

// int countTrailingZeros(unsigned ...):
//     Counts the number of trailing zeros in an unsigned integer type.
//     The result of countTrailingZeros(0) is undefined for all types.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_ctz) && VXIO_HAS_BUILTIN(__builtin_ctzl) && \
    VXIO_HAS_BUILTIN(__builtin_ctzll)
#define VXIO_HAS_BUILTIN_CTZ
constexpr int countTrailingZeros(unsigned char x) noexcept
{
    return __builtin_ctz(x);
}

constexpr int countTrailingZeros(unsigned short x) noexcept
{
    return __builtin_ctz(x);
}

constexpr int countTrailingZeros(unsigned int x) noexcept
{
    return __builtin_ctz(x);
}

constexpr int countTrailingZeros(unsigned long x) noexcept
{
    return __builtin_ctzl(x);
}

constexpr int countTrailingZeros(unsigned long long x) noexcept
{
    return __builtin_ctzll(x);
}
#endif

// int findFirstSet(unsigned ...):
//     Returns one plus the number of trailing zeros.
//     findFirstSet(0) evaluates to zero.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_ffs) && VXIO_HAS_BUILTIN(__builtin_ffsl) && \
    VXIO_HAS_BUILTIN(__builtin_ffsll)
#define VXIO_HAS_BUILTIN_FFS
constexpr int findFirstSet(unsigned char x) noexcept
{
    return __builtin_ffs(x);
}

constexpr int findFirstSet(unsigned short x) noexcept
{
    return __builtin_ffs(x);
}

constexpr int findFirstSet(unsigned int x) noexcept
{
    return __builtin_ffs(static_cast<int>(x));
}

constexpr int findFirstSet(unsigned long x) noexcept
{
    return __builtin_ffsl(static_cast<long>(x));
}

constexpr int findFirstSet(unsigned long long x) noexcept
{
    return __builtin_ffsll(static_cast<long long>(x));
}
#endif

// int parity(unsigned ...):
//     Returns the parity of a number.
//     This is a bool which indicates whether the number of set bits in x is odd.
//     The parity of 0 is 0, the parity of 1 is 1.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_parity) && VXIO_HAS_BUILTIN(__builtin_parityl) && \
    VXIO_HAS_BUILTIN(__builtin_parityll)
#define VXIO_HAS_BUILTIN_PARITY
constexpr bool parity(unsigned char x) noexcept
{
    return __builtin_parity(x);
}

constexpr int parity(unsigned short x) noexcept
{
    return __builtin_parity(x);
}

constexpr int parity(unsigned int x) noexcept
{
    return __builtin_parity(x);
}

constexpr int parity(unsigned long x) noexcept
{
    return __builtin_parityl(x);
}

constexpr int parity(unsigned long long x) noexcept
{
    return __builtin_parityll(x);
}
#endif

// int popCount(unsigned ...):
//     Counts the number of one-bits in an unsigned integer type.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_popcount) && VXIO_HAS_BUILTIN(__builtin_popcountl) && \
    VXIO_HAS_BUILTIN(__builtin_popcountll)
#define VXIO_HAS_BUILTIN_POPCOUNT
constexpr int popCount(unsigned char x) noexcept
{
    return __builtin_popcount(x);
}

constexpr int popCount(unsigned short x) noexcept
{
    return __builtin_popcount(x);
}

constexpr int popCount(unsigned int x) noexcept
{
    return __builtin_popcount(x);
}

constexpr int popCount(unsigned long x) noexcept
{
    return __builtin_popcountl(x);
}

constexpr int popCount(unsigned long long x) noexcept
{
    return __builtin_popcountll(x);
}
#elif defined(VXIO_MSVC)
constexpr uint8_t popCount(uint8_t x) noexcept
{
    return __popcnt16(x);
}

constexpr uint16_t popCount(uint16_t x) noexcept
{
    return __popcnt16(x);
}

constexpr uint32_t popCount(uint32_t x) noexcept
{
    return __popcnt32(x);
}

constexpr uint64_t popCount(uint64_t x) noexcept
{
    return __popcnt64(x);
}
#endif

// unsigned rotr(unsigned ...):
//     Right-rotates the bits of a number.
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_rotateright8) && VXIO_HAS_BUILTIN(__builtin_rotateright64)
#define VXIO_HAS_BUILTIN_ROTR
inline uint8_t rotr(uint8_t x, unsigned char rot) noexcept
{
    return __builtin_rotateright8(x, rot);
}

inline uint16_t rotr(uint16_t x, unsigned char rot) noexcept
{
    return __builtin_rotateright16(x, rot);
}

inline uint32_t rotr(uint32_t x, unsigned char rot) noexcept
{
    return __builtin_rotateright32(x, rot);
}

inline uint64_t rotr(uint64_t x, unsigned char rot) noexcept
{
    return __builtin_rotateright64(x, rot);
}
#endif

// unsigned rotl(unsigned ...):
//     Left-rotates the bits of a number.
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_rotateleft8) && VXIO_HAS_BUILTIN(__builtin_rotateleft64)
#define VXIO_HAS_BUILTIN_ROTL
inline uint8_t rotl(uint8_t x, unsigned char rot) noexcept
{
    return __builtin_rotateleft8(x, rot);
}

inline uint16_t rotl(uint16_t x, unsigned char rot) noexcept
{
    return __builtin_rotateleft16(x, rot);
}

inline uint32_t rotl(uint32_t x, unsigned char rot) noexcept
{
    return __builtin_rotateleft32(x, rot);
}

inline uint64_t rotl(uint64_t x, unsigned char rot) noexcept
{
    return __builtin_rotateleft64(x, rot);
}
#endif

// uintXX_t byteSwap(uintXX_t ...):
//     Swaps the bytes of any uintXX type.
//     This reverses the byte order (little-endian/big-endian).
//     This does nothing for byteSwap(uint8_t).
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_bswap16) && VXIO_HAS_BUILTIN(__builtin_bswap32) && \
    VXIO_HAS_BUILTIN(__builtin_bswap64)
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

}  // namespace voxelio::builtin

#endif  // BUILTIN_HPP
