#include "voxelio/format/png.hpp"

#include "voxelio/3rd_party/lodepngfwd.hpp"

#include "voxelio/log.hpp"
#include "voxelio/stream.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "voxelio/3rd_party/stb_image.h"

namespace voxelio::png {

static constexpr LodePNGColorType pngColorTypeOf(ColorFormat format)
{
    VXIO_ASSERT_NE(format, ColorFormat::ARGB32);
    switch (format) {
    case ColorFormat::V1: return LodePNGColorType::LCT_GREY;
    case ColorFormat::V8: return LodePNGColorType::LCT_GREY;
    case ColorFormat::VA16: return LodePNGColorType::LCT_GREY_ALPHA;
    case ColorFormat::RGB24: return LodePNGColorType::LCT_RGB;
    case ColorFormat::RGBA32: return LodePNGColorType::LCT_RGBA;
    case ColorFormat::ARGB32: return LodePNGColorType::LCT_RGBA;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

static constexpr ColorFormat colorFormatOfChannels(usize channels)
{
    switch (channels) {
    case 1: return ColorFormat::V8;
    case 2: return ColorFormat::VA16;
    case 3: return ColorFormat::RGB24;
    case 4: return ColorFormat::RGBA32;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

ResultCode encode(const Image &image, OutputStream &out)
{
    const auto width = static_cast<unsigned>(image.width());
    const auto height = static_cast<unsigned>(image.height());
    const auto type = pngColorTypeOf(image.format());
    const auto bitDepth = static_cast<unsigned>(bitDepthOf(image.format()));

    unsigned error = lodepng::encode(out, image.data(), width, height, type, bitDepth);
    if (error != 0) {
        VXIO_LOG(ERROR, lodepng_error_text(error));
        return ResultCode::WRITE_ERROR;
    }
    return ResultCode::OK;
}

static u8 BUFFER[8192];

std::optional<Image> decode(InputStream &in, usize desiredChannels, std::string &outError)
{
    // read stream fully
    ByteArrayOutputStream byteStream;
    while (in.good()) {
        usize bytesRead = in.read(BUFFER, sizeof(BUFFER));
        byteStream.write(BUFFER, bytesRead);
    }
    if (in.err()) {
        return std::nullopt;
    }

    int w, h, channels;
    u8 *image = stbi_load_from_memory(
        byteStream.data(), static_cast<int>(byteStream.size()), &w, &h, &channels, static_cast<int>(desiredChannels));

    if (image == nullptr) {
        outError = stbi_failure_reason();
        return std::nullopt;
    }

    const usize width = static_cast<usize>(w);
    const usize height = static_cast<usize>(h);
    // we always get desiredChannels, the channels variable only indicates how many channels are stored in the file
    const usize channelCount = desiredChannels == 0 ? static_cast<usize>(channels) : desiredChannels;
    const usize imageBytes = width * height * channelCount;
    const ColorFormat colorFormat = colorFormatOfChannels(channelCount);

    // this memcpy process is necessary because the image has been allocated with malloc()
    auto imageUptr = std::make_unique<u8[]>(imageBytes);
    std::memcpy(imageUptr.get(), image, imageBytes);
    stbi_image_free(image);

    return Image{width, height, colorFormat, std::move(imageUptr)};
}

}  // namespace voxelio::png
