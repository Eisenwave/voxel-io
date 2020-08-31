add_library(voxelio-format STATIC
    binvox.cpp
    binvox.hpp
    cubeworld.cpp
    cubeworld.hpp
    png.cpp
    png.hpp
    qb.cpp
    qb.hpp
    qef.cpp
    qef.hpp
    svx.cpp
    svx.hpp
    vl32.cpp
    vl32.hpp
    vobj.cpp
    vobj.hpp
    vox.cpp
    vox.hpp)

get_filename_component(PARENT_DIR ../ ABSOLUTE)
target_include_directories(voxelio-format PUBLIC "${PARENT_DIR}")
