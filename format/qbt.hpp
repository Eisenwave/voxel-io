#ifndef VXIO_QBT_HPP
#define VXIO_QBT_HPP

#if false
#include "endian_convert.hpp"
#include "vec.hpp"
#include "voxelio/types.hpp"

#include <istream>

static_assert (std::numeric_limits<float>::is_iec559);


void LoadNode(std::istream &stream);
void loadModel(std::istream &stream);
void loadCompound(std::istream &stream);

static Vec3f globalScale;

bool LoadQB2(std::istream &stream) {
  // Load Header
  u32 magic = read_big<u32>(stream);
  u8 major = stream.readByte;
  u8 minor = stream.readByte;

  if  (magic != 0x32204251u)
    return false;

  globalScale.x() = read_big<f32>(stream);
  globalScale.y() = read_big<f32>(stream);
  globalScale.z() = read_big<f32>(stream);

  // Load Color Map
  std::string colormapMagic = stream.readString(8); // = COLORMAP
  u32 colorCount = read_big<u32>(stream);
  for (u32 i = 0; i < colorCount; i++)
    color[i] = stream.readRGBA;

  // Load Data Tree
  std::string datatreeMagic = stream.readString(8); // = DATATREE
  LoadNode(stream);
}

void LoadNode(std::istream &stream) {
  u32 nodeTypeID = read_big<u32>(stream);
  u32 dataSize = read_big<u32>(stream);

  switch nodeTypeID {
    case 0:
      loadMatrix(stream);
    case 1:
      loadModel(stream);
    case 2:
      loadCompound(stream);
    else
      stream.seek(dataSize)
  }
}

void loadModel(std::istream &stream) {
  u32 childCount = stream.loadUInt;
  for (u32 i = 0; i < childCount; i++)
    loadNode(stream);
}

void loadMatrix(std::istream &stream) {
    Vec3i32 position;
    Vec3i32 localScale;
    Vec3f pivot;
    Vec3u32 size;

    u32 nameLength = read_big<u32>(stream);
    std::string name = stream.readString(nameLength);
    position.x() = read_big<i32>(stream);
    position.y() = read_big<i32>(stream);
    position.z() = read_big<i32>(stream);
    localScale.x() = read_big<i32>(stream);
    localScale.y() = read_big<i32>(stream);
    localScale.z() = read_big<i32>(stream);
    pivot.x() = read_big<f32>(stream);
    pivot.y() = read_big<f32>(stream);
    pivot.z() = read_big<f32>(stream);
    size.x() = read_big<u32>(stream);
    size.y() = read_big<u32>(stream);
    size.z() = read_big<u32>(stream);
    auto decompressStream = zlibDecompressStream(stream);
    for (u32 x = 0; x < size.x(); x++)
      for (u32 z = 0; z < size.z(); z++)
         for (u32 y = 0; y < size.y(); y++)
            voxelGrid[x,y,z] = decompressStream.ReadBuffer(4);
}

void loadCompound(std::istream &stream) {
    Vec3i32 position;
    Vec3i32 localScale;
    Vec3f pivot;
    Vec3u32 size;

    u32 nameLength = read_big<u32>(stream);
    std::string name = stream.readString(nameLength);
    position.x() = read_big<i32>(stream);
    position.y() = read_big<i32>(stream);
    position.z() = read_big<i32>(stream);
    localScale.x() = read_big<i32>(stream);
    localScale.y() = read_big<i32>(stream);
    localScale.z() = read_big<i32>(stream);
    pivot.x() = read_big<f32>(stream);
    pivot.y() = read_big<f32>(stream);
    pivot.z() = read_big<f32>(stream);
    size.x() = read_big<u32>(stream);
    size.y() = read_big<u32>(stream);
    size.z() = read_big<u32>(stream);

    auto decompressStream = new zlibDecompressStream(stream);
    for (u32 x = 0; x < size.x(); x++)
    for (u32 z = 0; z < size.z(); z++)
      for (u32 y = 0; y < size.y(); y++)
        voxelGrid[x,y,z] = decompressStream.ReadBuffer(4);

    u32 childCount = stream.loadUInt;
    if (mergeCompounds) { // if you don't need the datatree you can skip child nodes
    for (u32 i = 0; i < childCount; i++)
      skipNode(stream);
    }
    else {
    for (i = 0; i < childCount; i++)
      LoadNode(stream);
    }
}

void skipNode(std::istream &stream) {
  u32 nodeType = read_big<i32>(stream); // node type, can be ignored
  u32 dataSize = read_big<u32>(stream);
  stream.seekg(dataSize, std::istream::cur);
}
#endif

#endif  // VOXELIO_QBT_HPP
