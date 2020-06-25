# voxel-io

**voxel-io** is a C++17, MIT-licensed library for reading and writing various voxel formats.
It also comes with various useful C++ utilities for math, bit-manipulation, logging, asserts and more.

**voxel-io** is lightweight in the sense that STL-includes are heavily optimized and only used where absolutely necessary.
No third party dependencies exist, only single-header libraries (**lodepng**, **miniz-cpp**) were used.

## Top-Level Structure

All sources and headers can be found in `src/`.
`src/format` contains the readers/writers for various voxel formats.
`src/3rd_party` contains third-party libraries, which are all single-header libraries.

## Build

Right now, only QMake is supported.
CMake support is in development.`
