add_subdirectory(format)
add_subdirectory(3rd_party)

set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(VOXELIO_INCLUDE_DIR "${INCLUDE_DIR}/voxelio")

set(SOURCES
    assert.cpp
    builtin.cpp
    deflate.cpp
    filetype.cpp
    hextree_.cpp
    image.cpp
    ioutil.cpp
    log.cpp
    palette.cpp
    sstreamwrap.cpp
    stream.cpp
    stringify.cpp
    stringmanip.cpp
    voxelarray.cpp
    voxelio.cpp
    "${VOXELIO_INCLUDE_DIR}/assert.hpp"
    "${VOXELIO_INCLUDE_DIR}/bitcount.hpp"
    "${VOXELIO_INCLUDE_DIR}/bits.hpp"
    "${VOXELIO_INCLUDE_DIR}/bufstream.hpp"
    "${VOXELIO_INCLUDE_DIR}/build.hpp"
    "${VOXELIO_INCLUDE_DIR}/builtin.hpp"
    "${VOXELIO_INCLUDE_DIR}/color.hpp"
    "${VOXELIO_INCLUDE_DIR}/constantsfwd.hpp"
    "${VOXELIO_INCLUDE_DIR}/cstringview.hpp"
    "${VOXELIO_INCLUDE_DIR}/deflate.hpp"
    "${VOXELIO_INCLUDE_DIR}/endian.hpp"
    "${VOXELIO_INCLUDE_DIR}/filetype.hpp"
    "${VOXELIO_INCLUDE_DIR}/fstream.hpp"
    "${VOXELIO_INCLUDE_DIR}/hextree_.hpp"
    "${VOXELIO_INCLUDE_DIR}/ileave.hpp"
    "${VOXELIO_INCLUDE_DIR}/image.hpp"
    "${VOXELIO_INCLUDE_DIR}/intdiv.hpp"
    "${VOXELIO_INCLUDE_DIR}/intlog.hpp"
    "${VOXELIO_INCLUDE_DIR}/ioutil.hpp"
    "${VOXELIO_INCLUDE_DIR}/langext.hpp"
    "${VOXELIO_INCLUDE_DIR}/log.hpp"
    "${VOXELIO_INCLUDE_DIR}/macro.hpp"
    "${VOXELIO_INCLUDE_DIR}/palette.hpp"
    "${VOXELIO_INCLUDE_DIR}/parse.hpp"
    "${VOXELIO_INCLUDE_DIR}/primitives.hpp"
    "${VOXELIO_INCLUDE_DIR}/results.hpp"
    "${VOXELIO_INCLUDE_DIR}/sstreamwrap.hpp"
    "${VOXELIO_INCLUDE_DIR}/stream.hpp"
    "${VOXELIO_INCLUDE_DIR}/streamfwd.hpp"
    "${VOXELIO_INCLUDE_DIR}/stringify.hpp"
    "${VOXELIO_INCLUDE_DIR}/stringmanip.hpp"
    "${VOXELIO_INCLUDE_DIR}/types.hpp"
    "${VOXELIO_INCLUDE_DIR}/util.hpp"
    "${VOXELIO_INCLUDE_DIR}/vec.hpp"
    "${VOXELIO_INCLUDE_DIR}/voxelarray.hpp"
    "${VOXELIO_INCLUDE_DIR}/voxelio.hpp"
    "${VOXELIO_INCLUDE_DIR}/wbits.hpp"
    "${VOXELIO_INCLUDE_DIR}/wileave.hpp")

add_library(voxelio STATIC "${SOURCES}")
target_link_libraries(voxelio INTERFACE voxelio-format voxelio-3rd-party)
target_include_directories(voxelio PUBLIC "${INCLUDE_DIR}")
