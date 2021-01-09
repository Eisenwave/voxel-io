TARGET = voxelio
TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17 strict_c strict_c++

CONFIG(release, debug|release) {
    DEFINES += VXIO_RELEASE
}
CONFIG(debug, debug|release) {
    DEFINES += VXIO_DEBUG
}

LIBS += -lstdc++fs

INCLUDEPATH += include

SOURCES +=  \
    src/3rd_party/lodepng.cpp \
    src/3rd_party/miniz_cpp.cpp \
    src/3rd_party/miniz.c \
    src/assert.cpp \
    src/builtin.cpp \
    src/deflate.cpp \
    src/filetype.cpp \
    src/format/cubeworld.cpp \
    src/format/png.cpp \
    src/image.cpp \
    src/ioutil.cpp \
    src/log.cpp \
    src/sstreamwrap.cpp \
    src/stream.cpp \
    src/format/svx.cpp \
    src/stringmanip.cpp \
    src/voxelarray.cpp \
    src/voxelio.cpp \
    src/format/binvox.cpp \
    src/stringify.cpp \
    src/palette.cpp \
    src/format/qb.cpp \
    src/format/qef.cpp \
    src/format/vl32.cpp \
    src/format/vobj.cpp \
    src/format/vox.cpp

HEADERS +=  \
    include/voxelio/3rd_party/lodepng/lodepng.hpp \
    include/voxelio/3rd_party/lodepng/lodepngfwd.hpp \
    include/voxelio/3rd_party/miniz_cpp/miniz_cpp.hpp \
    include/voxelio/3rd_party/miniz_cpp/miniz_cppfwd.hpp \
    include/voxelio/3rd_party/miniz/miniz.h \
    include/voxelio/assert.hpp \
    include/voxelio/bits.hpp \
    include/voxelio/build.hpp \
    include/voxelio/builtin.hpp \
    include/voxelio/color.hpp \
    include/voxelio/constantsfwd.hpp \
    include/voxelio/deflate.hpp \
    include/voxelio/endian.hpp \
    include/voxelio/filetype.hpp \
    include/voxelio/format/cubeworld.hpp \
    include/voxelio/format/png.hpp \
    include/voxelio/ileave.hpp \
    include/voxelio/image.hpp \
    include/voxelio/intdiv.hpp \
    include/voxelio/intlog.hpp \
    include/voxelio/ioutil.hpp \
    include/voxelio/log.hpp \
    include/voxelio/macro.hpp \
    include/voxelio/parse.hpp \
    include/voxelio/primitives.hpp \
    include/voxelio/sstreamwrap.hpp \
    include/voxelio/stream.hpp \
    include/voxelio/format/svx.hpp \
    include/voxelio/streamfwd.hpp \
    include/voxelio/stringmanip.hpp \
    include/voxelio/util.hpp \
    include/voxelio/vec.hpp \
    include/voxelio/stringify.hpp \
    include/voxelio/voxelarray.hpp \
    include/voxelio/voxelio.hpp \
    include/voxelio/format/binvox.hpp \
    include/voxelio/palette.hpp \
    include/voxelio/format/qb.hpp \
    include/voxelio/format/qbt.hpp \
    include/voxelio/format/qef.hpp \
    include/voxelio/results.hpp \
    include/voxelio/types.hpp \
    include/voxelio/format/vl32.hpp \
    include/voxelio/format/vobj.hpp \
    include/voxelio/format/vox.hpp \
    include/voxelio/wbits.hpp \
    include/voxelio/wileave.hpp
