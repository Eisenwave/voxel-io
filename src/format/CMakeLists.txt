add_library(voxelio-format STATIC
    binvox.cpp
    cubeworld.cpp
    png.cpp
    qb.cpp
    qef.cpp
    svx.cpp
    vl32.cpp
    vobj.cpp
    vox.cpp)

target_include_directories(voxelio-format PUBLIC "${voxel-io_SOURCE_DIR}/include")
