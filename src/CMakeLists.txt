add_subdirectory(format)
add_subdirectory(3rd_party)

set(SOURCES
    assert.cpp
    builtin.cpp
    deflate.cpp
    filetype.cpp
    image.cpp
    ioutil.cpp
    log.cpp
    palette.cpp
    sstreamwrap.cpp
    stream.cpp
    stringify.cpp
    stringmanip.cpp
    voxelarray.cpp
    voxelio.cpp)

add_library(voxelio STATIC "${SOURCES}")
target_link_libraries(voxelio-format INTERFACE voxelio)
target_link_libraries(voxelio INTERFACE voxelio-format voxelio-3rd-party)
target_include_directories(voxelio PUBLIC "${voxel-io_SOURCE_DIR}/include")
