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

INCLUDEPATH += src

SOURCES +=  \
    src/3rd_party/lodepng/lodepng.cpp \
    src/3rd_party/miniz_cpp/miniz_cpp.cpp \
    src/3rd_party/miniz/miniz.c \
    src/assert.cpp \
    src/builtin.cpp \
    src/filetype.cpp \
    src/format/cubeworld.cpp \
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
    src/3rd_party/lodepng/lodepng.hpp \
    src/3rd_party/lodepng/lodepngfwd.hpp \
    src/3rd_party/miniz_cpp/miniz_cpp.hpp \
    src/3rd_party/miniz_cpp/miniz_cppfwd.hpp \
    src/3rd_party/miniz/miniz.h \
    src/assert.hpp \
    src/bits.hpp \
    src/build.hpp \
    src/builtin.hpp \
    src/color.hpp \
    src/constantsfwd.hpp \
    src/endian.hpp \
    src/filetype.hpp \
    src/format/cubeworld.hpp \
    src/ileave.hpp \
    src/image.hpp \
    src/intdiv.hpp \
    src/intlog.hpp \
    src/ioutil.hpp \
    src/log.hpp \
    src/macro.hpp \
    src/parse.hpp \
    src/primitives.hpp \
    src/sstreamwrap.hpp \
    src/stream.hpp \
    src/format/svx.hpp \
    src/streamfwd.hpp \
    src/stringmanip.hpp \
    src/util.hpp \
    src/vec.hpp \
    src/stringify.hpp \
    src/voxelarray.hpp \
    src/voxelio.hpp \
    src/format/binvox.hpp \
    src/palette.hpp \
    src/format/qb.hpp \
    src/format/qbt.hpp \
    src/format/qef.hpp \
    src/results.hpp \
    src/types.hpp \
    src/format/vl32.hpp \
    src/format/vobj.hpp \
    src/format/vox.hpp \
    src/wbits.hpp \
    src/wileave.hpp \
    src/zlibencode.hpp
