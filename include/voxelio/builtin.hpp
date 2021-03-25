#ifndef VXIO_BUILTIN_HPP
#define VXIO_BUILTIN_HPP
/*
 * builtin.hpp
 * -----------
 * Forwards to builtin functions in a compiler-independent way.
 */

#include "build.hpp"

#ifdef VXIO_MSVC
#include <intrin.h>
#endif

#if defined(VXIO_X86_OR_X64) && defined(VXIO_GNU_OR_CLANG)
#include <immintrin.h>
#endif

#include <cstdint>
#include <type_traits>

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

// METAPROGRAMMING =====================================================================================================

// VXIO_UNREACHABLE():
//     Signals to the compiler that a given code point is unreachable.
//     This macro is always defined, but does nothing if no builtin is available.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_unreachable)
#define VXIO_HAS_BUILTIN_UNREACHABLE
#define VXIO_UNREACHABLE() __builtin_unreachable()

#elif defined(VXIO_MSVC)
#define VXIO_HAS_BUILTIN_UNREACHABLE
#define VXIO_UNREACHABLE() __assume(false)

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
#define VXIO_ASSUME(condition) (static_cast<bool>(condition) ? void(0) : __builtin_unreachable())

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
// clang-format off
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_is_constant_evaluated) || \
    defined(VXIO_GNU) && VXIO_GNU >= 9 || \
    defined(VXIO_MSVC) && VXIO_MSVC >= 1925
// clang-format on
#define VXIO_HAS_BUILTIN_IS_CONSTANT_EVALUATED
constexpr bool isConstantEvaluated() noexcept
{
    return __builtin_is_constant_evaluated();
}
#endif

// BIT COUNTING ========================================================================================================

// int countLeadingRedundantSignBits(unsigned ...):
//     Counts the number of redundant sign bits, i.o.w. the number of bits following the sign bit which are equal to it.
//     This is the number of leading zeros minus one for positive numbers.
//     For negative numbers, it is the number of leading ones - 1.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_clrsb) && VXIO_HAS_BUILTIN(__builtin_clrsbl) && \
    VXIO_HAS_BUILTIN(__builtin_clrsbll)
#define VXIO_HAS_BUILTIN_CLRSB
inline int countLeadingRedundantSignBits(char x) noexcept
{
    return __builtin_clrsb(x);
}

inline int countLeadingRedundantSignBits(short x) noexcept
{
    return __builtin_clrsb(x);
}

inline int countLeadingRedundantSignBits(int x) noexcept
{
    return __builtin_clrsb(x);
}

inline int countLeadingRedundantSignBits(long x) noexcept
{
    return __builtin_clrsbl(x);
}

inline int countLeadingRedundantSignBits(long long x) noexcept
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
inline int countLeadingZeros(unsigned char x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned char)) * 8};
}

inline int countLeadingZeros(unsigned short x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned short)) * 8};
}

inline int countLeadingZeros(unsigned int x) noexcept
{
    return __builtin_clz(x);
}

inline int countLeadingZeros(unsigned long x) noexcept
{
    return __builtin_clzl(x);
}

inline int countLeadingZeros(unsigned long long x) noexcept
{
    return __builtin_clzll(x);
}

#elif defined(VXIO_MSVC) && defined(VXIO_X86_OR_X64)
#define VXIO_HAS_BUILTIN_CLZ
__forceinline int countLeadingZeros(unsigned char x) noexcept
{
    return __lzcnt16(x) - 8;
}

__forceinline int countLeadingZeros(unsigned short x) noexcept
{
    return __lzcnt16(x);
}

__forceinline int countLeadingZeros(unsigned int x) noexcept
{
    return __lzcnt(x);
}

__forceinline int countLeadingZeros(unsigned long x) noexcept
{
    static_assert(sizeof(unsigned long) == sizeof(unsigned) || sizeof(unsigned long) == sizeof(uint64_t));

    if constexpr (sizeof(unsigned long) == sizeof(unsigned)) {
        return __lzcnt(static_cast<unsigned>(x));
    }
    else {
        return __lzcnt64(static_cast<uint64_t>(x));
    }
}

#ifdef VXIO_64_BIT
__forceinline int countLeadingZeros(uint64_t x) noexcept
{
    return __lzcnt64(x);
}
#else
[[noreturn]] __forceinline int countLeadingZeros(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int countTrailingZeros(unsigned ...):
//     Counts the number of trailing zeros in an unsigned integer type.
//     The result of countTrailingZeros(0) is undefined for all types.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_ctz) && VXIO_HAS_BUILTIN(__builtin_ctzl) && \
    VXIO_HAS_BUILTIN(__builtin_ctzll)
#define VXIO_HAS_BUILTIN_CTZ
inline int countTrailingZeros(unsigned char x) noexcept
{
    return __builtin_ctz(x);
}

inline int countTrailingZeros(unsigned short x) noexcept
{
    return __builtin_ctz(x);
}

inline int countTrailingZeros(unsigned int x) noexcept
{
    return __builtin_ctz(x);
}

inline int countTrailingZeros(unsigned long x) noexcept
{
    return __builtin_ctzl(x);
}

inline int countTrailingZeros(unsigned long long x) noexcept
{
    return __builtin_ctzll(x);
}
#endif

// int leastSignificantBit(unsigned ...):
//     Returns the index of the least significant bit.
//     For example, leastSignificantBit(0b110) -> 1.
//     The result of leastSignificantBit(0) is 0 for all types.
#ifdef VXIO_MSVC
#define VXIO_HAS_BUILTIN_LSB
__forceinline int leastSignificantBit(unsigned char x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int leastSignificantBit(unsigned short x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int leastSignificantBit(unsigned int x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int leastSignificantBit(unsigned long x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

#ifdef VXIO_64_BIT
__forceinline int leastSignificantBit(uint64_t x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward64(&result, x);
    return nonzero * static_cast<int>(result);
}
#else
[[noreturn]] __forceinline int leastSignificantBit(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int mostSignificantBit(unsigned ...):
//     Returns the index of the most significant bit.
//     For example, mostSignificantBit(0b110) -> 2.
//     The result of mostSignificantBit(0) is 0 for all types.
#ifdef VXIO_MSVC
#define VXIO_HAS_BUILTIN_MSB
__forceinline int mostSignificantBit(unsigned char x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int mostSignificantBit(unsigned short x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int mostSignificantBit(unsigned int x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int mostSignificantBit(unsigned long x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

#ifdef VXIO_64_BIT
__forceinline int mostSignificantBit(uint64_t x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse64(&result, x);
    return nonzero * static_cast<int>(result);
}
#else
[[noreturn]] __forceinline int mostSignificantBit(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int findFirstSet(unsigned ...):
//     Returns one plus the number of trailing zeros.
//     findFirstSet(0) evaluates to zero.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_ffs) && VXIO_HAS_BUILTIN(__builtin_ffsl) && \
    VXIO_HAS_BUILTIN(__builtin_ffsll)
#define VXIO_HAS_BUILTIN_FFS
inline int findFirstSet(unsigned char x) noexcept
{
    return __builtin_ffs(x);
}

inline int findFirstSet(unsigned short x) noexcept
{
    return __builtin_ffs(x);
}

inline int findFirstSet(unsigned int x) noexcept
{
    return __builtin_ffs(static_cast<int>(x));
}

inline int findFirstSet(unsigned long x) noexcept
{
    return __builtin_ffsl(static_cast<long>(x));
}

inline int findFirstSet(unsigned long long x) noexcept
{
    return __builtin_ffsll(static_cast<long long>(x));
}
#endif

// int popCount(unsigned ...):
//     Counts the number of one-bits in an unsigned integer type.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_popcount) && VXIO_HAS_BUILTIN(__builtin_popcountl) && \
    VXIO_HAS_BUILTIN(__builtin_popcountll)
#define VXIO_HAS_BUILTIN_POPCOUNT
inline int popCount(unsigned char x) noexcept
{
    return __builtin_popcount(x);
}

inline int popCount(unsigned short x) noexcept
{
    return __builtin_popcount(x);
}

inline int popCount(unsigned int x) noexcept
{
    return __builtin_popcount(x);
}

inline int popCount(unsigned long x) noexcept
{
    return __builtin_popcountl(x);
}

inline int popCount(unsigned long long x) noexcept
{
    return __builtin_popcountll(x);
}
#elif defined(VXIO_MSVC)
#define VXIO_HAS_BUILTIN_POPCOUNT
__forceinline int popCount(uint8_t x) noexcept
{
    return static_cast<int>(__popcnt16(x));
}

__forceinline int popCount(uint16_t x) noexcept
{
    return static_cast<int>(__popcnt16(x));
}

__forceinline int popCount(uint32_t x) noexcept
{
    return static_cast<int>(__popcnt(x));
}

#ifdef VXIO_64_BIT
__forceinline int popCount(uint64_t x) noexcept
{
    return static_cast<int>(__popcnt64(x));
}
#else
[[noreturn]] __forceinline int popCount(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int parity(unsigned ...):
//     Returns the parity of a number.
//     This is a bool which indicates whether the number of set bits in x is odd.
//     The parity of 0 is 0, the parity of 1 is 1.
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_parity) && VXIO_HAS_BUILTIN(__builtin_parityl) && \
    VXIO_HAS_BUILTIN(__builtin_parityll)
#define VXIO_HAS_BUILTIN_PARITY
inline bool parity(unsigned char x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned short x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned int x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned long x) noexcept
{
    return __builtin_parityl(x);
}

inline bool parity(unsigned long long x) noexcept
{
    return __builtin_parityll(x);
}
#endif

// ADVANCED BITWISE OPS ================================================================================================

// unsigned rotr(unsigned ...):
//     Right-rotates the bits of a number.
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_rotateright8) && VXIO_HAS_BUILTIN(__builtin_rotateright64)
#define VXIO_HAS_BUILTIN_ROTR
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint rotateRight(Uint x, unsigned char rot) noexcept
{
    if constexpr (sizeof(x) == 1) return __builtin_rotateright8(static_cast<uint8_t>(x), rot);
    if constexpr (sizeof(x) == 2) return __builtin_rotateright16(static_cast<uint16_t>(x), rot);
    if constexpr (sizeof(x) == 4) return __builtin_rotateright32(static_cast<uint32_t>(x), rot);
    if constexpr (sizeof(x) == 8) return __builtin_rotateright64(static_cast<uint64_t>(x), rot);
}
#endif

// unsigned rotl(unsigned ...):
//     Left-rotates the bits of a number.
#if defined(VXIO_CLANG) && VXIO_HAS_BUILTIN(__builtin_rotateleft8) && VXIO_HAS_BUILTIN(__builtin_rotateleft64)
#define VXIO_HAS_BUILTIN_ROTL
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint rotateLeft(Uint x, unsigned char rot) noexcept
{
    if constexpr (sizeof(x) == 1) return __builtin_rotateleft8(static_cast<uint8_t>(x), rot);
    if constexpr (sizeof(x) == 2) return __builtin_rotateleft16(static_cast<uint16_t>(x), rot);
    if constexpr (sizeof(x) == 4) return __builtin_rotateleft32(static_cast<uint32_t>(x), rot);
    if constexpr (sizeof(x) == 8) return __builtin_rotateleft64(static_cast<uint64_t>(x), rot);
}
#endif

// uintXX_t byteSwap(uintXX_t ...):
//     Swaps the bytes of any uintXX type.
//     This reverses the byte order (little-endian/big-endian).
//     This does nothing for byteSwap(uint8_t).
#if defined(VXIO_GNU_OR_CLANG) && VXIO_HAS_BUILTIN(__builtin_bswap16) && VXIO_HAS_BUILTIN(__builtin_bswap32) && \
    VXIO_HAS_BUILTIN(__builtin_bswap64)
#define VXIO_HAS_BUILTIN_BSWAP
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint byteSwap(Uint x) noexcept
{
    if constexpr (sizeof(x) == 1) return x;
    if constexpr (sizeof(x) == 2) return __builtin_bswap16(x);
    if constexpr (sizeof(x) == 4) return __builtin_bswap32(x);
    if constexpr (sizeof(x) == 8) return __builtin_bswap64(x);
}

#elif defined(VXIO_MSVC)
#define VXIO_HAS_BUILTIN_BSWAP
__forceinline uint8_t byteSwap(uint8_t val) noexcept
{
    return val;
}

__forceinline unsigned short byteSwap(unsigned short val) noexcept
{
    return _byteswap_ushort(val);
}

__forceinline unsigned int byteSwap(unsigned int val) noexcept
{
    static_assert(sizeof(unsigned int) == sizeof(unsigned short) || sizeof(unsigned int) == sizeof(unsigned long),
                  "No viable _byteswap implementation for unsigned int");
    if constexpr (sizeof(unsigned int) == sizeof(unsigned short)) {
        return static_cast<unsigned int>(_byteswap_ushort(static_cast<unsigned short>(val)));
    }
    else {
        return static_cast<unsigned int>(_byteswap_ulong(static_cast<unsigned long>(val)));
    }
}

__forceinline unsigned long byteSwap(unsigned long val) noexcept
{
    return _byteswap_ulong(val);
}

#ifdef VXIO_64_BIT
__forceinline uint64_t byteSwap(uint64_t val) noexcept
{
    return _byteswap_uint64(val);
}
#else
[[noreturn]] __forceinline uint64_t byteSwap(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// BMI2 BITWISE OPS ====================================================================================================

// uintXX_t depositBits(uintXX_t val, uintXX_t mask):
//     See https://www.felixcloutier.com/x86/pdep
#if defined(VXIO_X86_OR_X64) && (defined(VXIO_MSVC) || defined(VXIO_GNU_OR_CLANG))
#define VXIO_HAS_BUILTIN_PDEP
template <typename Uint, std::enable_if_t<(std::is_unsigned_v<Uint> && sizeof(Uint) <= 8), int> = 0>
Uint depositBits(Uint val, Uint mask)
{
    if constexpr (sizeof(Uint) < 8) {
        return _pdep_u32(static_cast<uint32_t>(val), static_cast<uint32_t>(mask));
    }
    else {
        return _pdep_u64(val, mask);
    }
}

// uintXX_t extractBits(uintXX_t val, uintXX_t mask):
//     See https://www.felixcloutier.com/x86/pext
#define VXIO_HAS_BUILTIN_PEXT
template <typename Uint, std::enable_if_t<(std::is_unsigned_v<Uint> && sizeof(Uint) <= 8), int> = 0>
Uint extractBits(Uint val, Uint mask)
{
    if constexpr (sizeof(Uint) < 8) {
        return _pext_u32(static_cast<uint64_t>(val), static_cast<uint64_t>(mask));
    }
    else {
        return _pext_u64(val, mask);
    }
}
#endif

}  // namespace voxelio::builtin

#endif  // BUILTIN_HPP
