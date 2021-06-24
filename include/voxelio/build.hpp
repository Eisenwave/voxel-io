#ifndef VXIO_BUILD_HPP
#define VXIO_BUILD_HPP
/*
 * build.hpp
 * -----------
 * Captures information about the build, such as the C++ standard, compiler, debug/release build, etc.
 *
 * This header does not and should never have additional includes.
 * See https://godbolt.org/z/xza5qY for testing.
 */

// BUILD TYPE DETECTION ================================================================================================

#if !defined(VXIO_DEBUG) && !defined(VXIO_RELEASE)
#error "Either VXIO_DEBUG or VXIO_RELEASE must be defined"
#endif

// expressions which get removed by the preprocessor if they are not on the right type of build
// e.g. IF_DEBUG(expr) removes expr on all builds that are *not* debug builds
#ifdef VXIO_DEBUG
#define VXIO_IF_RELEASE(expr)
#define VXIO_IF_DEBUG(expr) expr
#else
#define VXIO_IF_RELEASE(expr) expr
#define VXIO_IF_DEBUG(expr)
#endif

// compiles to t one debug builds and to f on release builds
#define VXIO_IF_DEBUG_ELSE(t, f) VXIO_IF_DEBUG(t) VXIO_IF_RELEASE(f)

namespace voxelio::build {

constexpr bool DEBUG = VXIO_IF_DEBUG_ELSE(true, false);
constexpr bool RELEASE = !DEBUG;

}  // namespace voxelio::build

// C++ STANDARD DETECTION ==============================================================================================

#if __cplusplus >= 199711L
#define VXIO_CPP98_LEAST
#endif
#if __cplusplus >= 201103L
#define VXIO_CPP11_LEAST
#endif
#if __cplusplus >= 201402L
#define VXIO_CPP14_LEAST
#endif
#if __cplusplus >= 201703L
#define VXIO_CPP17_LEAST
#endif
#if __cplusplus >= 202002L
#define VXIO_CPP20_LEAST
#endif

#if __cplusplus == 199711L
#define VXIO_CPP98
#elif __cplusplus == 201103L
#define VXIO_CPP11
#elif __cplusplus == 201402L
#define VXIO_CPP14
#elif __cplusplus == 201703L
#define VXIO_CPP17
#elif __cplusplus == 202002L
#define VXIO_CPP20
#endif

// COMPILER DETECTION ==================================================================================================

#ifdef _MSC_VER
#define VXIO_MSVC _MSC_VER
#endif

#ifdef __GNUC__
#define VXIO_GNU_OR_CLANG
#endif

#ifdef __clang__
#define VXIO_CLANG __clang_major__
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define VXIO_GNU __GNUC__
#endif

// Ensure that alternative operator keywords like not, and, etc. exist for MSVC (they don't by default).
// In C++20, the <ciso646> header was removed from the standard.
#if defined(VXIO_MSVC) && !defined(VXIO_CPP20_LEAST)
#include <ciso646>
#endif

#ifdef VXIO_GNU_OR_CLANG
#define VXIO_FWDHEADER(header) <bits/header##fwd.h>
#else
#define VXIO_FWDHEADER(header) <header>
#endif

// ARCH DETECTION ======================================================================================================

#ifdef __i386__
#define VXIO_X86
#endif

#ifdef __x86_64__
#define VXIO_X64
#endif

#ifdef _M_IX86
#define VXIO_X86
#endif

#ifdef _M_AMD64
#define VXIO_X64
#endif

#ifdef _M_ARM64
#define VXIO_ARM64
#endif

#if defined(VXIO_X86) || defined(VXIO_X64)
#define VXIO_X86_OR_X64
#endif

#if defined(VXIO_X64) || defined(VXIO_ARM64)
#define VXIO_64_BIT
#endif

// OS DETECTION ========================================================================================================

#ifdef __unix__
#define VXIO_UNIX
#endif

// ENDIANNES DETECTION =================================================================================================
/*
 * In this section, the bool constant NATIVE_ENDIAN_LITTLE is defined.
 * This happens either using __BYTE_ORDER__ macros or C++20's std::endian.
 */

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__
// C++17 ENDIANNESS DETECTION USING __BYTE__ORDER MACRO ----------------------------------------------------------------

namespace voxelio::build {

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr bool NATIVE_ENDIAN_LITTLE = true;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
constexpr bool NATIVE_ENDIAN_LITTLE = false;
#elif __BYTE_ORDER == __ORDER_PDP_ENDIAN__
#error "voxelio can't compile on platforms with PDP endianness"
#else
#error "__BYTE_ORDER__ has unrecognized value"
#endif

}  // namespace voxelio::build

#elif VXIO_CPP20_LEAST
// C++20 ENDIANNESS DETECTION ------------------------------------------------------------------------------------------
#include <bit>

namespace voxelio::build {

constexpr bool NATIVE_ENDIAN_LITTLE = std::endian::native == std::endian::little;

}  // namespace voxelio::build

#elif defined(VXIO_X86_OR_X64)
// ARCHITECTURE BASED ENDIANNESS DETECTION -----------------------------------------------------------------------------

namespace voxelio::build {

constexpr bool NATIVE_ENDIAN_LITTLE = true;

}  // namespace voxelio::build

#else
#error "Failed to detect platform endianness"
#endif

// ENDIAN ENUM =========================================================================================================

namespace voxelio::build {

/**
 * @brief Represents a byte order. Can be either Big Endian or Little Endian.
 */
enum class Endian : unsigned {
    /// Least significant byte first.
    LITTLE = 0,
    /// Most significant byte first.
    BIG = 1,
    NATIVE = NATIVE_ENDIAN_LITTLE ? LITTLE : BIG
};

}  // namespace voxelio::build

// SOURCE LOCATION =====================================================================================================

namespace voxelio {

struct SourceLocation {
    const char *file;
    const char *function;
    unsigned long line;
};

}  // namespace voxelio

#endif  // VXIO_BUILD_HPP
