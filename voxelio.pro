TARGET = voxelio
TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17 strict_c strict_c++

DEFINES += VOXELIO_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(release, debug|release) {
    DEFINES += VXIO_RELEASE
}
CONFIG(debug, debug|release) {
    DEFINES += VXIO_DEBUG
}

SOURCES +=  \
    3rd_party/lodepng.cpp \
    3rd_party/miniz_cpp.cpp \
    assert.cpp \
    builtin.cpp \
    format/cubeworld.cpp \
    image.cpp \
    log.cpp \
    sstreamwrap.cpp \
    stream.cpp \
    format/svx.cpp \
    stringmanip.cpp \
    voxelarray.cpp \
    voxelio.cpp \
    format/binvox.cpp \
    constants.cpp \
    stringify.cpp \
    palette.cpp \
    format/qb.cpp \
    format/qef.cpp \
    util_public.cpp \
    format/vl32.cpp \
    format/vobj.cpp \
    format/vox.cpp

HEADERS +=  \
    3rd_party/lodepng.hpp \
    3rd_party/lodepngfwd.hpp \
    3rd_party/miniz.hpp \
    3rd_party/miniz_cpp.hpp \
    3rd_party/miniz_cppfwd.hpp \
    assert.hpp \
    build.hpp \
    builtin.hpp \
    color.hpp \
    endian.hpp \
    format/cubeworld.hpp \
    image.hpp \
    intdiv.hpp \
    log.hpp \
    macro.hpp \
    parse.hpp \
    sstreamwrap.hpp \
    stream.hpp \
    format/svx.hpp \
    stringmanip.hpp \
    vec.hpp \
    log2.hpp \
    stringify.hpp \
    voxelarray.hpp \
    voxelio.hpp \
    format/binvox.hpp \
    constants.hpp \
    palette.hpp \
    format/qb.hpp \
    format/qbt.hpp \
    format/qef.hpp \
    results.hpp \
    types.hpp \
    util_public.hpp \
    format/vl32.hpp \
    format/vobj.hpp \
    format/vox.hpp
