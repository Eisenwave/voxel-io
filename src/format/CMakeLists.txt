set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(INCLUDE_FORMAT_DIR "${INCLUDE_DIR}/voxelio/format")

add_library(voxelio-format STATIC
    binvox.cpp
    cubeworld.cpp
    ply.cpp
    png.cpp
    qb.cpp
    qef.cpp
    svx.cpp
    vl32.cpp
    vobj.cpp
    vox.cpp
    xyzrgb.cpp
    "${INCLUDE_FORMAT_DIR}/binvox.hpp"
    "${INCLUDE_FORMAT_DIR}/cubeworld.hpp"
    "${INCLUDE_FORMAT_DIR}/ply.hpp"
    "${INCLUDE_FORMAT_DIR}/png.hpp"
    "${INCLUDE_FORMAT_DIR}/qb.hpp"
    "${INCLUDE_FORMAT_DIR}/qbt.hpp"
    "${INCLUDE_FORMAT_DIR}/qef.hpp"
    "${INCLUDE_FORMAT_DIR}/svx.hpp"
    "${INCLUDE_FORMAT_DIR}/vl32.hpp"
    "${INCLUDE_FORMAT_DIR}/vobj.hpp"
    "${INCLUDE_FORMAT_DIR}/vox.hpp"
    "${INCLUDE_FORMAT_DIR}/xyzrgb.hpp")

target_include_directories(voxelio-format PUBLIC "${INCLUDE_DIR}")
target_link_libraries(voxelio-format INTERFACE voxelio)
