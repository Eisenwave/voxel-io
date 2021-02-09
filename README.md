# voxel-io

**voxel-io** is a C++17, MIT-licensed library for reading and writing various voxel formats.
It also comes with various useful C++ utilities for math, bit-manipulation, logging, asserts and more.

**voxel-io** is lightweight in the sense that STL-includes are heavily optimized and only used where absolutely necessary.
No third party dependencies exist, only single-header libraries (**lodepng**, **miniz-cpp**) were used.

## Top-Level Structure

All sources can be found in `src/`.
Headers can be found in `include`.
`src/format` contains the readers/writers for various voxel formats.

`src/3rd_party` contains third-party libraries.
These are implementation details and using them through voxel-io is not safe.
They might be changed by voxel-io or they might be removed entirely in the future.

## Build

To build using CMake:
```sh
mkdir build
cd build
cmake .. # -DCMAKE_BUILD_TYPE=DEBUG for debug builds, otherwise release is default
make # -j NUMBER_OF_THREADS
```

**voxel-io** should build in about 1-2 seconds with multithreading.
