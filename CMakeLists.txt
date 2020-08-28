cmake_minimum_required(VERSION 3.2)
project(voxel-io)
#add_executable(voxel-io helloworld.cpp) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SOURCE_DIR src)

string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)

if(build_type STREQUAL DEBUG)
    message("Building voxel-io DEBUG")
    add_definitions(-DVXIO_DEBUG)
else()
    message("Building voxel-io RELEASE")
    add_definitions(-DVXIO_RELEASE)
endif()

add_subdirectory(src)
