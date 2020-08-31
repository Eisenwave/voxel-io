add_library(voxelio-3rd-party STATIC
    lodepng/lodepng.cpp
    lodepng/lodepng.hpp
    miniz/miniz.c
    miniz/miniz.h
    miniz_cpp/miniz_cpp.cpp
    miniz_cpp/miniz_cpp.hpp)

get_filename_component(PARENT_DIR ../ ABSOLUTE)
target_include_directories(voxelio-3rd-party PUBLIC "${PARENT_DIR}")
