get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR})

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
