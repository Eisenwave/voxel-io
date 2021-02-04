set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(INCLUDE_3RD_PARTY_DIR "${INCLUDE_DIR}/voxelio/3rd_party")

add_library(voxelio-3rd-party STATIC
    lodepng.cpp
    miniz.c
    miniz_cpp.cpp
    "${INCLUDE_3RD_PARTY_DIR}/lodepng.hpp"
    "${INCLUDE_3RD_PARTY_DIR}/lodepngfwd.hpp"
    "${INCLUDE_3RD_PARTY_DIR}/miniz.h"
    "${INCLUDE_3RD_PARTY_DIR}/miniz_cpp.hpp"
    "${INCLUDE_3RD_PARTY_DIR}/miniz_cppfwd.hpp"
    "${INCLUDE_3RD_PARTY_DIR}/stb_image.h")

target_include_directories(voxelio-3rd-party PUBLIC "${INCLUDE_DIR}")
