#ifndef VXIO_VOXOBJ_HPP
#define VXIO_VOXOBJ_HPP

#include "voxelio/types.hpp"
#include "voxelio/voxelio.hpp"

#include <stack>
#include <unordered_set>

namespace voxelio::vobj {

enum ColorFormat : u8 { RGB24 = 0x18, ARGB32 = 0x20, V8 = 0x48, AV16 = 0x50 };

enum DataFormat : u8 { EMPTY = 0x10, LIST = 0x20, ARRAY_POSITIONED = 0x30, ARRAY_TILED = 0x31 };

constexpr const char *EXT_DEBUG = "debug";
constexpr const char *EXT_GROUPS = "group";
constexpr const char *EXT_EXISTENCE_ARRAY = "exArr";
constexpr const char *EXT_16_BIT_ARRAY = "arr16";
constexpr const char *EXT_32_BIT_ARRAY = "arr32";

using ArrayU8 = std::unique_ptr<u8[]>;

struct GroupHeader {
    std::string name;
    Vec3i32 pos;
};

struct GroupNode {
    GroupHeader group;
    std::unordered_set<std::string> childNames{};

    explicit GroupNode(GroupHeader group) : group{std::move(group)} {}
};

class VobjWriteHelper {
private:
    Voxel64 *buffer;
    size_t bufferSize;

    size_t index;

    Vec3i64 offset = Vec3i64::zero();
    ColorFormat colorFormat;

public:
    class OffsetGuard {
    private:
        VobjWriteHelper *parent;
        Vec3i64 offset = Vec3i64::zero();

    public:
        OffsetGuard(VobjWriteHelper *parent, Vec3i64 offset);

        ~OffsetGuard();
    };

    size_t voxelsWritten();
    void setBaseOffset(Vec3i64 offset);
    [[nodiscard]] OffsetGuard addGuardedOffset(Vec3i64 offset);
    void setColorFormat(ColorFormat format);
    void resetBuffer(Voxel64 buffer[], size_t bufferSize);
    bool canWrite();
    bool isFull();
    void write(Vec3i64 pos, argb32 color);
};

class Reader : public AbstractReader {
private:
    struct State {
        ArrayU8 existArr;
        Vec3i64 arrPos;
        Vec3u64 arrDims;
        u64 arrIndex, arrLim;
        u32 arrX, arrY, arrZ;
        u32 grpIndex, grpLim;
        u32 datIndex, datLim;
        DataFormat format;
    };

    struct Extensions {
        bool debug : 1;
        bool exArr : 1;
        bool group : 1;
        bool arr16 : 1;
        bool arr32 : 1;
    };

    struct Palette {
        u8 bits;
        u64 size;
        ArrayU8 content;
    };

    VobjWriteHelper writeHelper;

    std::stack<GroupNode> groupStack{};

    bool initialized = false;
    Extensions ext;

    ColorFormat colorFormat;
    u8 colorByteCount;

    Palette palette;
    State state;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen}
    {
        groupStack.emplace(GroupHeader{"", Vec3i32::zero()});
    }

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;

private:
    bool pushGroup(GroupHeader group);
    bool popGroups(size_t count);

    argb32 decodeColor(u8 data[]);

    // IO

    [[nodiscard]] ReadResult readString(size_t length, std::string &out);
    [[nodiscard]] ReadResult readString(std::string &out);
    [[nodiscard]] ReadResult skipString();
    [[nodiscard]] ReadResult readArrayU8(size_t length, ArrayU8 &out);

    [[nodiscard]] ReadResult readHeader();
    [[nodiscard]] ReadResult readColorFormat();
    [[nodiscard]] ReadResult readExtensions();
    [[nodiscard]] ReadResult readPalette();
    [[nodiscard]] ReadResult readDataFormat();
    [[nodiscard]] ReadResult readDimensions();

    [[nodiscard]] ReadResult readVoxel(argb32 &out);
    [[nodiscard]] ReadResult readVoxelUsingPalette(argb32 &out);

    [[nodiscard]] ReadResult readContent(bool resume);
    [[nodiscard]] ReadResult readGroup(bool resume);
    [[nodiscard]] ReadResult readTypedData(bool resume);
    [[nodiscard]] ReadResult readPositionedVoxel(bool resume);
    [[nodiscard]] ReadResult readPositionedArray(bool resume);
    [[nodiscard]] ReadResult readTiledArray(bool resume);
    [[nodiscard]] ReadResult readArrayContent(bool resume);
    [[nodiscard]] ReadResult readArrayWithContentExistence(bool resume);
    [[nodiscard]] ReadResult readArrayContentWithoutExistence(bool resume);
};

}  // namespace voxelio::vobj

#endif  // VOXOBJ_HPP
