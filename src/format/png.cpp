#include "png.hpp"

#include "../3rd_party/lodepng/lodepngfwd.hpp"

#include "log.hpp"

namespace voxelio::png {

static constexpr LodePNGColorType pngColorTypeOf(ColorFormat format)
{
    VXIO_ASSERT_NE(format, ColorFormat::ARGB32);
    switch (format) {
    case ColorFormat::V1: return LodePNGColorType::LCT_GREY;
    case ColorFormat::V8: return LodePNGColorType::LCT_GREY;
    case ColorFormat::RGB24: return LodePNGColorType::LCT_RGB;
    case ColorFormat::RGBA32: return LodePNGColorType::LCT_RGBA;
    case ColorFormat::ARGB32: return LodePNGColorType::LCT_RGBA;
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

}  // namespace voxelio::png
