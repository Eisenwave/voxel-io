#ifndef VXIO_VOX_HPP
#define VXIO_VOX_HPP

#include "voxelio/ioutil.hpp"
#include "voxelio/voxelio.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

namespace voxelio::vox {

constexpr size_t CHUNK_NAME_LENGTH = 4;
constexpr size_t PALETTE_SIZE = 256;

#define REGISTER_CHUNK_TYPE(name) name = (#name[0] << 24) | (#name[1] << 16) | (#name[2] << 8) | #name[3]

enum class ChunkType : u32 {
    // BASE
    REGISTER_CHUNK_TYPE(MAIN),
    REGISTER_CHUNK_TYPE(SIZE),
    REGISTER_CHUNK_TYPE(XYZI),
    REGISTER_CHUNK_TYPE(RGBA),
    REGISTER_CHUNK_TYPE(MATT),
    REGISTER_CHUNK_TYPE(PACK),

    // EXTENDED
    REGISTER_CHUNK_TYPE(nGRP),
    REGISTER_CHUNK_TYPE(nSHP),
    REGISTER_CHUNK_TYPE(nTRN),

    REGISTER_CHUNK_TYPE(LAYR),
    REGISTER_CHUNK_TYPE(MATL),

    // UNDOCUMENTED
    REGISTER_CHUNK_TYPE(IMAP),  // I don't fucking know
    REGISTER_CHUNK_TYPE(rOBJ)   // renderer settings
};

#undef REGISTER_CHUNK_TYPE

static_assert(static_cast<unsigned>(ChunkType::MAIN) == 0x4d41494e);

constexpr std::array<ChunkType, 13> CHUNK_TYPE_VALUES{ChunkType::MAIN,
                                                      ChunkType::SIZE,
                                                      ChunkType::XYZI,
                                                      ChunkType::RGBA,
                                                      ChunkType::MATT,
                                                      ChunkType::PACK,
                                                      ChunkType::nGRP,
                                                      ChunkType::nSHP,
                                                      ChunkType::nTRN,
                                                      ChunkType::LAYR,
                                                      ChunkType::MATL,
                                                      ChunkType::IMAP,
                                                      ChunkType::rOBJ};

enum class NodeType { TRANSFORM, GROUP, SHAPE };

struct SceneNode {
    NodeType type;
    /** Polymorphic content ID. The index of the voxel chunk for SHAPE nodes, the index of the transform for
     * TRANSFORM nodes and 0 for group nodes. */
    u32 contentId;
};

struct ChunkHeader;

struct Transformation {
    static Transformation concat(const Transformation &lhs, const Transformation &rhs);

    std::array<Vec3i8, 3> matrix{Vec3i8{i8{1}, i8{0}, i8{0}}, Vec3i8{i8{0}, i8{1}, i8{0}}, {i8{0}, i8{0}, i8{1}}};
    Vec3i32 translation{};

    Vec3i8 row(size_t index) const;
    Vec3i8 col(size_t index) const;
    Vec3i32 apply(const Vec3u32 &pointInChunk, const Vec3u32 &doublePivot) const;
    std::string toString() const;
};

struct VoxelChunkInfo {
    Vec3u32 size;
    u32 voxelCount;
    u64 pos;
    std::vector<u32> parentIds{};

    std::string toString() const;
};

class Reader : public AbstractReader {
private:
    struct State {
        ChunkType previousChunkType;
        size_t modelIndex = 0;
        size_t parentIndex = 0;
        size_t voxelIndex = 0;
        Transformation transform;
    };

    using dict_t = std::unordered_map<std::string, std::string>;

    std::unique_ptr<argb32[]> palette = std::make_unique<argb32[]>(PALETTE_SIZE);
    VoxelBufferWriteHelper writeHelper;

    std::multimap<u32, u32> nodeParentMap;
    std::map<u32, SceneNode> nodeMap;
    std::vector<VoxelChunkInfo> voxelChunkInfos;
    std::vector<Transformation> transformations;
    std::vector<u32> shapeNodeIds;

    State state;
    u32 rootNodeId = 0;
    bool initialized = false;

public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;
    [[nodiscard]] ReadResult read(Voxel32 buffer[], size_t bufferLength);
    float progress() noexcept final;

private:
    [[nodiscard]] ReadResult processSceneGraph() noexcept;
    [[nodiscard]] ReadResult expectChars(const char name[CHUNK_NAME_LENGTH]) noexcept;
    [[nodiscard]] ReadResult readString(std::string &out) noexcept;
    [[nodiscard]] ReadResult readDict(dict_t &out) noexcept;

    [[nodiscard]] ReadResult skipChunk() noexcept;
    [[nodiscard]] ReadResult skipString() noexcept;
    [[nodiscard]] ReadResult skipDict() noexcept;

    [[nodiscard]] ReadResult doRead() noexcept;
    [[nodiscard]] ReadResult readMagicAndVersion() noexcept;
    [[nodiscard]] ReadResult readTransformationDict(Transformation &out) noexcept;
    [[nodiscard]] ReadResult decodeRotation(u8 in, Transformation &out) noexcept;
    [[nodiscard]] ReadResult readOneVoxel(const Vec3u32 &chunkSize) noexcept;

    [[nodiscard]] ReadResult readChunk(bool isEofAtFirstByteAllowed) noexcept;
    [[nodiscard]] ReadResult readChunkHeader(bool isEofAtFirstByteAllowed, ChunkHeader &out) noexcept;
    [[nodiscard]] ReadResult readChunkType(ChunkType &out) noexcept;
    [[nodiscard]] ReadResult readChunkContent(const ChunkHeader &header) noexcept;
    [[nodiscard]] ReadResult readChunkContent_main() noexcept;
    [[nodiscard]] ReadResult readChunkContent_size() noexcept;
    [[nodiscard]] ReadResult readChunkContent_xyzi() noexcept;
    [[nodiscard]] ReadResult readChunkContent_rgba() noexcept;
    [[nodiscard]] ReadResult readChunkContent_nodeTransform() noexcept;
    [[nodiscard]] ReadResult readChunkContent_nodeGroup() noexcept;
    [[nodiscard]] ReadResult readChunkContent_nodeShape() noexcept;
    [[nodiscard]] ReadResult readChunkContent_layer() noexcept;

    [[nodiscard]] ReadResult makeError_expectedButGot(const std::string &field, i64 expected, i64 got) noexcept;

    void updateTransformForCurrentShape();
};

}  // namespace voxelio::vox

#endif  // READER_HPP
