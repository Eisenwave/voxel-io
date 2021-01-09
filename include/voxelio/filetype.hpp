#ifndef VXIO_FILETYPE_HPP
#define VXIO_FILETYPE_HPP
/*
 * filetype.hpp
 * -----------
 * Provides various file-type related constants such as format category, magic bytes, etc.
 */

#include "constantsfwd.hpp"

#include "assert.hpp"

#include <array>
#include <optional>

namespace voxelio {

enum class FileType : unsigned {
    /** Binvox voxel model. */
    BINVOX = 0,
    /** Microsoft Bitmap image.  */
    MICROSOFT_BITMAP = 1,
    /** CompuServe Graphics Interchange Format. */
    GRAPHICS_INTERCHANGE = 2,
    /** Joint Pictures Expert Group image. */
    JPEG_IMAGE = 3,
    /** Portable Network Graphics image. */
    PORTABLE_NETWORK_GRAPHICS = 4,
    /** Qubicle Binary model. */
    QUBICLE_BINARY = 5,
    /** Qubicle Binary Tree model. Successor to Qubicle Binary. */
    QUBICLE_BINARY_TREE = 6,
    /** Qubicle Exchange Format. Plaintext alternative to Qubicle Binary. */
    QUBICLE_EXCHANGE = 7,
    /** 3D Systems Stereolithography triangle mesh. */
    STEREOLITHOGRAPHY = 8,
    /** Wavefront polygonal mesh. */
    WAVEFRONT_OBJ = 9,
    /** MagicaVoxel voxel model. */
    MAGICA_VOX = 10,
    /** MassVoxelEditor Voxel Object. */
    MVE_VOBJ = 11,
    /** 32-bit voxel list. */
    VL32 = 12,
    /** Comma-separated values. */
    COMMA_SEPARATED_VALUES = 13,
    /** Markdown text. */
    MARKDOWN = 14,
    /** JavaScript object notation. */
    JSON = 15,
    /** Java properties configuration file. */
    JAVA_PROPERTIES = 16,
    /** Plaintext file. */
    PLAINTEXT = 17,
    /** OpenVDB voxel database. */
    OPENVDB_VDB = 18,
    /** Shapeways Simple Voxels model. */
    SIMPLE_VOXELS = 19,
    /** PKWare ZIP archive. */
    PKWARE_ZIP = 20,
    /** Stanford triangle Format, aka. Polygon File Format (PLY). */
    STANFORD_TRIANGLE = 21,
    /** KV6 voxel model. */
    KV6 = 22,
    /** KVX voxel model. */
    KVX = 23,
    /** SLAB6 voxel model. */
    SLAB6_VOX = 24,
    /** Cubeworld CUB model. */
    CUBEWORLD_CUB = 25,
    /** Minecraft schematic model. */
    MINECRAFT_SCHEMATIC = 26,
    /** Minecraft structure model. */
    MINECRAFT_STRUCTURE = 27,
    /** Minecraft region model. */
    MINECRAFT_REGION = 28,
    /** Ace of Spades voxel model. */
    ACE_OF_SPADES_VXL = 29,
    /** SLABSPRI voxel model. */
    SLABSPRI_VOX = 30,
    /** Paint3D 3MP model. */
    PAINT3D_3MP = 31,
    /** Zoxel voxel model. */
    ZOXEL = 32,
    /** Free lossless voxel compression. */
    FLVC = 33
};

enum class FileTypeCategory : unsigned { VOXEL, MESH, IMAGE, TEXT, ARCHIVE, POINT_CLOUD };

enum class FileTypeStructure : unsigned { BINARY, TEXT, BINARY_WITH_TEXT_HEADER, MIXED };

constexpr std::array<FileType, 34> FILE_TYPE_VALUES{FileType::BINVOX,
                                                    FileType::MICROSOFT_BITMAP,
                                                    FileType::GRAPHICS_INTERCHANGE,
                                                    FileType::JPEG_IMAGE,
                                                    FileType::PORTABLE_NETWORK_GRAPHICS,
                                                    FileType::QUBICLE_BINARY,
                                                    FileType::QUBICLE_BINARY_TREE,
                                                    FileType::QUBICLE_EXCHANGE,
                                                    FileType::STEREOLITHOGRAPHY,
                                                    FileType::WAVEFRONT_OBJ,
                                                    FileType::MVE_VOBJ,
                                                    FileType::MAGICA_VOX,
                                                    FileType::VL32,
                                                    FileType::COMMA_SEPARATED_VALUES,
                                                    FileType::MARKDOWN,
                                                    FileType::JSON,
                                                    FileType::JAVA_PROPERTIES,
                                                    FileType::PLAINTEXT,
                                                    FileType::OPENVDB_VDB,
                                                    FileType::SIMPLE_VOXELS,
                                                    FileType::PKWARE_ZIP,
                                                    FileType::STANFORD_TRIANGLE,
                                                    FileType::KV6,
                                                    FileType::KVX,
                                                    FileType::SLAB6_VOX,
                                                    FileType::CUBEWORLD_CUB,
                                                    FileType::MINECRAFT_SCHEMATIC,
                                                    FileType::MINECRAFT_STRUCTURE,
                                                    FileType::MINECRAFT_REGION,
                                                    FileType::ACE_OF_SPADES_VXL,
                                                    FileType::SLABSPRI_VOX,
                                                    FileType::PAINT3D_3MP,
                                                    FileType::ZOXEL,
                                                    FileType::FLVC};

constexpr std::array<FileTypeCategory, 6> FILE_TYPE_CATEGORY_VALUES{FileTypeCategory::VOXEL,
                                                                    FileTypeCategory::MESH,
                                                                    FileTypeCategory::IMAGE,
                                                                    FileTypeCategory::TEXT,
                                                                    FileTypeCategory::ARCHIVE,
                                                                    FileTypeCategory::POINT_CLOUD};

constexpr std::array<FileTypeStructure, 4> FILE_TYPE_STRUCTURE_VALUES{FileTypeStructure::BINARY,
                                                                      FileTypeStructure::TEXT,
                                                                      FileTypeStructure::BINARY_WITH_TEXT_HEADER,
                                                                      FileTypeStructure::MIXED};

constexpr const char *nameOf(FileType fileType)
{
#define VXIO_REGISTER(name) \
    case FileType::name: return #name
    switch (fileType) {
        VXIO_REGISTER(BINVOX);
        VXIO_REGISTER(MICROSOFT_BITMAP);
        VXIO_REGISTER(GRAPHICS_INTERCHANGE);
        VXIO_REGISTER(JPEG_IMAGE);
        VXIO_REGISTER(PORTABLE_NETWORK_GRAPHICS);
        VXIO_REGISTER(QUBICLE_BINARY);
        VXIO_REGISTER(QUBICLE_BINARY_TREE);
        VXIO_REGISTER(QUBICLE_EXCHANGE);
        VXIO_REGISTER(STEREOLITHOGRAPHY);
        VXIO_REGISTER(WAVEFRONT_OBJ);
        VXIO_REGISTER(MVE_VOBJ);
        VXIO_REGISTER(MAGICA_VOX);
        VXIO_REGISTER(VL32);
        VXIO_REGISTER(COMMA_SEPARATED_VALUES);
        VXIO_REGISTER(MARKDOWN);
        VXIO_REGISTER(JSON);
        VXIO_REGISTER(JAVA_PROPERTIES);
        VXIO_REGISTER(PLAINTEXT);
        VXIO_REGISTER(SIMPLE_VOXELS);
        VXIO_REGISTER(OPENVDB_VDB);
        VXIO_REGISTER(PKWARE_ZIP);
        VXIO_REGISTER(STANFORD_TRIANGLE);
        VXIO_REGISTER(KV6);
        VXIO_REGISTER(KVX);
        VXIO_REGISTER(SLAB6_VOX);
        VXIO_REGISTER(CUBEWORLD_CUB);
        VXIO_REGISTER(MINECRAFT_SCHEMATIC);
        VXIO_REGISTER(MINECRAFT_STRUCTURE);
        VXIO_REGISTER(MINECRAFT_REGION);
        VXIO_REGISTER(ACE_OF_SPADES_VXL);
        VXIO_REGISTER(SLABSPRI_VOX);
        VXIO_REGISTER(PAINT3D_3MP);
        VXIO_REGISTER(ZOXEL);
        VXIO_REGISTER(FLVC);
    }
    return "";
#undef VXIO_REGISTER
}

constexpr const char *nameOf(FileTypeCategory fileType)
{
    switch (fileType) {
    case FileTypeCategory::VOXEL: return "VOXEL";
    case FileTypeCategory::MESH: return "MESH";
    case FileTypeCategory::IMAGE: return "IMAGE";
    case FileTypeCategory::TEXT: return "TEXT";
    case FileTypeCategory::ARCHIVE: return "ARCHIVE";
    case FileTypeCategory::POINT_CLOUD: return "POINT_CLOUD";
    }
    return "";
}

constexpr const char *nameOf(FileTypeStructure fileType)
{
    switch (fileType) {
    case FileTypeStructure::BINARY: return "BINARY";
    case FileTypeStructure::TEXT: return "TEXT";
    case FileTypeStructure::BINARY_WITH_TEXT_HEADER: return "BINARY_WITH_TEXT_HEADER";
    case FileTypeStructure::MIXED: return "MIXED";
    }
    return "";
}

constexpr const char *extensionOf(FileType fileType)
{
    switch (fileType) {
    case FileType::BINVOX: return "binvox";
    case FileType::MICROSOFT_BITMAP: return "bmp";
    case FileType::GRAPHICS_INTERCHANGE: return "gif";
    case FileType::JPEG_IMAGE: return "jpeg";
    case FileType::PORTABLE_NETWORK_GRAPHICS: return "png";
    case FileType::QUBICLE_BINARY: return "qb";
    case FileType::QUBICLE_BINARY_TREE: return "qbt";
    case FileType::QUBICLE_EXCHANGE: return "qef";
    case FileType::STEREOLITHOGRAPHY: return "stl";
    case FileType::WAVEFRONT_OBJ: return "obj";
    case FileType::MVE_VOBJ: return "vobj";
    case FileType::MAGICA_VOX: return "vox";
    case FileType::VL32: return "vl32";
    case FileType::COMMA_SEPARATED_VALUES: return "csv";
    case FileType::MARKDOWN: return "md";
    case FileType::JSON: return "json";
    case FileType::JAVA_PROPERTIES: return "properties";
    case FileType::PLAINTEXT: return "txt";
    case FileType::OPENVDB_VDB: return "vdb";
    case FileType::SIMPLE_VOXELS: return "svx";
    case FileType::PKWARE_ZIP: return "zip";
    case FileType::STANFORD_TRIANGLE: return "ply";
    case FileType::KV6: return "kv6";
    case FileType::KVX: return "kvx";
    case FileType::SLAB6_VOX: return "vox";
    case FileType::CUBEWORLD_CUB: return "cub";
    case FileType::MINECRAFT_SCHEMATIC: return "schematic";
    case FileType::MINECRAFT_STRUCTURE: return "mcstruct";
    case FileType::MINECRAFT_REGION: return "mcr";
    case FileType::ACE_OF_SPADES_VXL: return "vxl";
    case FileType::SLABSPRI_VOX: return "vox";
    case FileType::PAINT3D_3MP: return "3mp";
    case FileType::ZOXEL: return "zox";
    case FileType::FLVC: return "flvc";
    }
    return "";
}

constexpr const char *alternativeExtensionOf(FileType fileType)
{
    switch (fileType) {
    case FileType::JPEG_IMAGE: return "jpg";
    default: return "";
    }
}

constexpr const char *displayNameOf(FileType fileType)
{
    switch (fileType) {
    case FileType::BINVOX: return "Binvox";
    case FileType::MICROSOFT_BITMAP: return "Microsoft Bitmap";
    case FileType::GRAPHICS_INTERCHANGE: return "Graphics Interchange Format";
    case FileType::JPEG_IMAGE: return "JPEG";
    case FileType::PORTABLE_NETWORK_GRAPHICS: return "Portable Network Graphics";
    case FileType::QUBICLE_BINARY: return "Qubicle Binary";
    case FileType::QUBICLE_BINARY_TREE: return "Qubicle Binary Tree";
    case FileType::QUBICLE_EXCHANGE: return "Qubicle Exchange Format";
    case FileType::STEREOLITHOGRAPHY: return "Stereolithography Model";
    case FileType::WAVEFRONT_OBJ: return "Wavefront Object";
    case FileType::MVE_VOBJ: return "MVE Voxel Object";
    case FileType::MAGICA_VOX: return "MagicaVoxel Model";
    case FileType::VL32: return "32-Bit Voxel List";
    case FileType::COMMA_SEPARATED_VALUES: return "Comma-Separated Values";
    case FileType::MARKDOWN: return "Markdown Document";
    case FileType::JSON: return "JavaScript Object Notation";
    case FileType::JAVA_PROPERTIES: return "Properties";
    case FileType::PLAINTEXT: return "Plaintext";
    case FileType::OPENVDB_VDB: return "OpenVDB Volume Database";
    case FileType::SIMPLE_VOXELS: return "Simple Voxels";
    case FileType::PKWARE_ZIP: return "Zip Archive";
    case FileType::STANFORD_TRIANGLE: return "Polygon";
    case FileType::KV6: return "SLAB6 KV6 Model";
    case FileType::KVX: return "KVX Model";
    case FileType::SLAB6_VOX: return "SLAB6 VOX Model";
    case FileType::CUBEWORLD_CUB: return "Cubeworld Model";
    case FileType::MINECRAFT_SCHEMATIC: return "Minecraft Schematic";
    case FileType::MINECRAFT_STRUCTURE: return "Minecraft Structure";
    case FileType::MINECRAFT_REGION: return "Minecraft Region";
    case FileType::ACE_OF_SPADES_VXL: return "Ace of Spades Map (Voxlap)";
    case FileType::SLABSPRI_VOX: return "SLABSPRI VOX";
    case FileType::PAINT3D_3MP: return "Paint3D 3MP";
    case FileType::ZOXEL: return "Zoxel";
    case FileType::FLVC: return "Free Lossless Voxel Compression";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr const char *mediaTypeOf(FileType fileType)
{
    switch (fileType) {
    case FileType::BINVOX: return "model/x-binvox";
    case FileType::MICROSOFT_BITMAP: return "image/bmp";
    case FileType::GRAPHICS_INTERCHANGE: return "image/gif";
    case FileType::JPEG_IMAGE: return "image/jpeg";
    case FileType::PORTABLE_NETWORK_GRAPHICS: return "image/png";
    case FileType::QUBICLE_BINARY: return "model/x-qb";
    case FileType::QUBICLE_BINARY_TREE: return "model/x-qbt";
    case FileType::QUBICLE_EXCHANGE: return "text/x-qef";
    case FileType::STEREOLITHOGRAPHY: return "model/stl";
    case FileType::WAVEFRONT_OBJ: return "model/obj";
    case FileType::MVE_VOBJ: return "model/x-vobj";
    case FileType::MAGICA_VOX: return "model/x-vox";
    case FileType::VL32: return "model/x-vl32";
    case FileType::COMMA_SEPARATED_VALUES: return "text/csv";
    case FileType::MARKDOWN: return "text/md";
    case FileType::JSON: return "application/json";
    case FileType::JAVA_PROPERTIES: return "text/x-properties+plain";
    case FileType::PLAINTEXT: return "text/plain";
    case FileType::OPENVDB_VDB: return "model/x-vdb";
    case FileType::SIMPLE_VOXELS: return "application/x-svx+zip";
    case FileType::PKWARE_ZIP: return "application/zip";
    case FileType::STANFORD_TRIANGLE: return "text/x-ply+plain";
    case FileType::KV6: return "model/x-kv6";
    case FileType::KVX: return "model/x-kv";
    case FileType::SLAB6_VOX: return "model/x-slab6-vox";
    case FileType::CUBEWORLD_CUB: return "model/x-cubeworld";
    case FileType::MINECRAFT_SCHEMATIC: return "application/x-schematic+nbt";
    case FileType::MINECRAFT_STRUCTURE: return "application/x-minecraft-structure+nbt";
    case FileType::MINECRAFT_REGION: return "application/x-minecraft-region";
    case FileType::ACE_OF_SPADES_VXL: return "model/x-vxl";
    case FileType::SLABSPRI_VOX: return "model/x-slabspri-vox";
    case FileType::PAINT3D_3MP: return "model/x-3mp";
    case FileType::ZOXEL: return "model/x-zoxel";
    case FileType::FLVC: return "model/x-flvc";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr const char *magicOf(FileType fileType)
{
    switch (fileType) {
    case FileType::BINVOX: return "#binvox";
    case FileType::MICROSOFT_BITMAP: return "BM";
    case FileType::GRAPHICS_INTERCHANGE: return "GIF\x38";  // GIF87a or GIF89a
    case FileType::JPEG_IMAGE: return "\xff\xd8\xff";       // 4 variants exist starting at the next byte
    case FileType::PORTABLE_NETWORK_GRAPHICS: return "\x89PNG\x0d\x0a\x1a\x0a";
    case FileType::QUBICLE_BINARY_TREE: return "QB 2";                  // ... oh look, he fixed it
    case FileType::QUBICLE_EXCHANGE: return "Qubicle Exchange Format";  // first line of mandatory copyright header
    case FileType::STEREOLITHOGRAPHY: return "solid";                   // only applies to the ASCII version of STL
    case FileType::MVE_VOBJ: return "model/x-vobj";
    case FileType::MAGICA_VOX: return "VOX ";  // the trailing space is intentional
    case FileType::PKWARE_ZIP:
    case FileType::SIMPLE_VOXELS: return "\x50\x4b";  // one can go more into detail to detect empty archives too
    case FileType::STANFORD_TRIANGLE: return "ply\n";
    case FileType::KV6: return "Kvxl";
    default: return "";
    }
}

constexpr FileTypeCategory categoryOf(FileType type)
{
    switch (type) {
    case FileType::MICROSOFT_BITMAP:
    case FileType::GRAPHICS_INTERCHANGE:
    case FileType::JPEG_IMAGE:
    case FileType::PORTABLE_NETWORK_GRAPHICS: return FileTypeCategory::IMAGE;

    case FileType::STANFORD_TRIANGLE:
    case FileType::STEREOLITHOGRAPHY:
    case FileType::WAVEFRONT_OBJ: return FileTypeCategory::MESH;

    case FileType::COMMA_SEPARATED_VALUES:
    case FileType::MARKDOWN:
    case FileType::JSON:
    case FileType::JAVA_PROPERTIES:
    case FileType::PLAINTEXT: return FileTypeCategory::TEXT;

    case FileType::PKWARE_ZIP: return FileTypeCategory::ARCHIVE;

    default: return FileTypeCategory::VOXEL;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr FileTypeStructure structureOf(FileType type)
{
    switch (type) {
    case FileType::BINVOX:
    case FileType::STANFORD_TRIANGLE: return FileTypeStructure::BINARY_WITH_TEXT_HEADER;

    case FileType::STEREOLITHOGRAPHY: return FileTypeStructure::MIXED;

    case FileType::QUBICLE_EXCHANGE:
    case FileType::WAVEFRONT_OBJ:
    case FileType::COMMA_SEPARATED_VALUES:
    case FileType::MARKDOWN:
    case FileType::JSON:
    case FileType::JAVA_PROPERTIES:
    case FileType::PLAINTEXT: return FileTypeStructure::TEXT;

    default: return FileTypeStructure::BINARY;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

/** Returns whether voxelio provides readers for the given file type by default. */
constexpr bool isReadableByDefault(FileType fileType)
{
    switch (fileType) {
    case FileType::BINVOX:
    case FileType::QUBICLE_BINARY:
    case FileType::QUBICLE_EXCHANGE:
    case FileType::MVE_VOBJ:
    case FileType::MAGICA_VOX:
    case FileType::VL32:
    case FileType::CUBEWORLD_CUB: return true;
    default: return false;
    }
}

/** Returns whether voxelio provides writers for the given file type by default. */
constexpr bool isWritableByDefault(FileType fileType)
{
    switch (fileType) {
    case FileType::QUBICLE_BINARY:
    case FileType::QUBICLE_EXCHANGE:
    case FileType::VL32:
    case FileType::SIMPLE_VOXELS: return true;
    default: return false;
    }
}

/**
 * @brief Returns whether a given voxel format supports the use of palettes.
 * (also called color maps by QBT documentation)
 *
 * @param fileType the file type
 * @return whether the type supports palettes
 */
constexpr bool supportsPalette(FileType fileType)
{
    switch (fileType) {
    case FileType::QUBICLE_EXCHANGE:
    case FileType::QUBICLE_BINARY_TREE:
    case FileType::MVE_VOBJ:
    case FileType::MAGICA_VOX:
    case FileType::KV6:
    case FileType::KVX: return true;
    default: return false;
    }
}

/**
 * @brief Returns whether a given voxel format requires the use of palettes.
 * (also called color maps by QBT documentation)
 *
 * @param fileType the file type
 * @return whether the type requires palettes
 */
constexpr bool requiresPalette(FileType fileType)
{
    switch (fileType) {
    case FileType::QUBICLE_EXCHANGE:
    case FileType::MAGICA_VOX:
    case FileType::KVX: return true;
    default: return false;
    }
}

struct FileTypeInfo {
    const char *name;
    const char *extension;
    const char *extensionAlt;
    const char *displayName;
    const char *mediaType;
    const char *magic;
    FileType type;
    FileTypeCategory category;
    FileTypeStructure structure;
    bool defaultReadable;
    bool defaultWritable;
    bool paletteSupported;
    bool paletteRequired;

    constexpr explicit FileTypeInfo(FileType fileType)
        : name{nameOf(fileType)}
        , extension{extensionOf(fileType)}
        , extensionAlt{alternativeExtensionOf(fileType)}
        , displayName{displayNameOf(fileType)}
        , mediaType{mediaTypeOf(fileType)}
        , magic{magicOf(fileType)}
        , type{fileType}
        , category{categoryOf(fileType)}
        , structure{structureOf(fileType)}
        , defaultReadable{isReadableByDefault(fileType)}
        , defaultWritable{isWritableByDefault(fileType)}
        , paletteSupported{supportsPalette(fileType)}
        , paletteRequired{requiresPalette(fileType)}
    {
    }

    FileTypeInfo(const FileTypeInfo &) = default;
    FileTypeInfo(FileTypeInfo &&) = default;
    FileTypeInfo &operator=(FileTypeInfo &&) = default;
    FileTypeInfo &operator=(const FileTypeInfo &) = default;
};

constexpr FileTypeInfo infoOf(FileType fileType)
{
    return FileTypeInfo{fileType};
}

/**
 * Tries to detect the type of a file using both its name and magic bytes.
 *
 * @param path the file path
 * @param out the file type output
 * @return whether the file type could be detected
 */
std::optional<FileType> detectFileType(const std::string &path);

std::optional<FileType> detectFileTypeUsingName(const std::string &path);

std::optional<FileType> detectFileTypeUsingMagic(const std::string &path);

}  // namespace voxelio

std::ostream &operator<<(std::ostream &stream, voxelio::FileType fileType);

#endif  // VXIO_CONSTANTS_HPP
