add_library(voxelio-3rd-party STATIC
    lodepng.cpp
    miniz.c
    miniz_cpp.cpp)

target_include_directories(voxelio-3rd-party PUBLIC "${voxel-io_SOURCE_DIR}/include")
