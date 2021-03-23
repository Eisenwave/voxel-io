cmake_minimum_required(VERSION 3.2)
project(voxel-io)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SOURCE_DIR src)

string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)

if(build_type STREQUAL DEBUG)
    message("Building voxel-io DEBUG")
    add_compile_definitions(VXIO_DEBUG)
elseif(build_type STREQUAL RELWITHDEBINFO)
    message("Building voxel-io RELEASE (with debug info)")
    add_compile_definitions(VXIO_RELEASE)
else()
    message("Building voxel-io RELEASE")
    add_compile_definitions(VXIO_RELEASE)
endif()

add_subdirectory(src)
add_subdirectory(test)
