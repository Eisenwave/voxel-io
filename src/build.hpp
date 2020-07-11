#ifndef VXIO_BUILD_HPP
#define VXIO_BUILD_HPP

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
// compiles to t on release builds and to f on debug builds
#define VXIO_IF_RELEASE_ELSE(t, f) VXIO_IF_DEBUG(f) VXIO_IF_RELEASE(t)

namespace voxelio::build {

constexpr bool DEBUG = VXIO_IF_DEBUG_ELSE(true, false);
constexpr bool RELEASE = VXIO_IF_RELEASE_ELSE(true, false);

static_assert(DEBUG != RELEASE, "DEBUG and RELEASE must be mutually exclusive");

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
#define VXIO_MSVC
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

// ENDIANNES DETECTION =================================================================================================

namespace voxelio::build {

enum class Endian : unsigned { BIG = 0, LITTLE = 1 };

}

#ifdef VXIO_CPP20_LEAST
// C++20 ENDIANNESS DETECTION ------------------------------------------------------------------------------------------
#define VXIO_HAS_KNOWN_ENDIAN
#include <bit>

namespace voxelio::build {

constexpr bool NATIVE_ENDIAN_KNOWN = true;
constexpr Endian NATIVE_ENDIAN = std::endian::native == std::endian::little ? Endian::LITTLE : Endian::BIG;

}  // namespace voxelio::build

#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__  // VXIO_CPP20_LEAST
// C++17 ENDIANNESS DETECTION USING __BYTE__ORDER MACRO ----------------------------------------------------------------
#define VXIO_HAS_KNOWN_ENDIAN

#define VXIO_ENDIAN_LITTLE 1234
#define VXIO_ENDIAN_BIG 4321
#define VXIO_ENDIAN_PDP 3412

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define VXIO_ENDIAN VXIO_ENDIAN_LITTLE
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define VXIO_ENDIAN VXIO_ENDIAN_BIG
#elif __BYTE_ORDER == __ORDER_PDP_ENDIAN__
#define VXIO_ENDIAN VXIO_ENDIAN_PDP
#endif

#if VXIO_ENDIAN == VXIO_ENDIAN_PDP
#error "voxelio can't compile on platforms with PDP endianness"
#endif

namespace voxelio::build {

constexpr bool NATIVE_ENDIAN_KNOWN = true;

#if VXIO_ENDIAN == VXIO_ENDIAN_LITTLE
constexpr Endian NATIVE_ENDIAN = Endian::LITTLE;
#elif VXIO_ENDIAN == VXIO_ENDIAN_BIG
constexpr bool NATIVE_ENDIAN = Endian::BIG;
#else
#error "__BYTE_ORDER__ has unrecognized value"
#endif

}  // namespace voxelio::build

#else
// C++17 UNKNWON ENDIANNESS --------------------------------------------------------------------------------------------
namespace voxelio::build {

constexpr bool NATIVE_ENDIAN_KNOWN = false;
constexpr Endian NATIVE_ENDIAN = static_cast<Endian>(0xff);

}  // namespace voxelio::build
#endif

#endif  // VXIO_BUILD_HPP
