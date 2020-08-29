get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR})

add_library(voxelio-3rd-party STATIC
    lodepng/lodepng.cpp
    miniz/miniz.c
    miniz_cpp/miniz_cpp.cpp)
