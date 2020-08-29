add_subdirectory(format)
add_subdirectory(3rd_party)

set(SOURCES assert.cpp
    builtin.cpp
    deflate.cpp
    filetype.cpp
    image.cpp
    ioutil.cpp
    log.cpp
    palette.cpp
    sstreamwrap.cpp
    stringify.cpp
    stringmanip.cpp
    voxelarray.cpp
    voxelio.cpp)

add_library(voxelio STATIC "${SOURCES}")
