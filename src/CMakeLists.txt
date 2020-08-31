add_subdirectory(format)
add_subdirectory(3rd_party)

set(SOURCES
    assert.cpp
    assert.hpp
    bits.hpp
    builtin.cpp
    builtin.hpp
    deflate.cpp
    deflate.hpp
    endian.hpp
    filetype.cpp
    filetype.hpp
    ileave.hpp
    image.cpp
    image.hpp
    intdiv.hpp
    intlog.hpp
    ioutil.cpp
    ioutil.hpp
    log.cpp
    log.hpp
    macro.hpp
    palette.cpp
    palette.hpp
    parse.hpp
    primitives.hpp
    results.hpp
    sstreamwrap.cpp
    sstreamwrap.hpp
    stream.cpp
    stream.hpp
    streamfwd.hpp
    stringify.cpp
    stringify.hpp
    stringmanip.cpp
    stringmanip.hpp
    types.hpp
    util.hpp
    vec.hpp
    voxelarray.cpp
    voxelarray.hpp
    voxelio.cpp
    voxelio.hpp
    wbits.hpp
    wileave.hpp)

add_library(voxelio STATIC "${SOURCES}")
target_link_libraries(voxelio-format INTERFACE voxelio)
target_link_libraries(voxelio INTERFACE voxelio-format voxelio-3rd-party)
