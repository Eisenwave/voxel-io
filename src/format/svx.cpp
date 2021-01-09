#include "voxelio/format/svx.hpp"

#include "voxelio/3rd_party/miniz_cpp.hpp"

#include "voxelio/format/png.hpp"

#include "voxelio/color.hpp"
#include "voxelio/image.hpp"
#include "voxelio/log.hpp"
#include "voxelio/macro.hpp"
#include "voxelio/stringmanip.hpp"

namespace voxelio::svx {

// STATIC UTILITY ======================================================================================================

namespace {

constexpr const char *xmlNameOf(ChannelType type)
{
    switch (type) {
    case ChannelType::DENSITY: return "DENSITY";
    case ChannelType::COLOR_RGB: return "COLOR-RGB";
    case ChannelType::MATERIAL: return "MATERIAL";
    case ChannelType::CUSTOM: return "CUSTOM";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr bool usesId(ChannelType type)
{
    return type == ChannelType::MATERIAL || type == ChannelType::CUSTOM;
}

struct SvxLimits {
    size_t slice;
    size_t u;
    size_t v;
};

/**
 * Transforms world space coordinates into slice, u, v coordinates.
 */
template <Axis AXIS>
constexpr SvxLimits svxLimitsOf(Vec3size xyzLimits)
{
    if constexpr (AXIS == Axis::X) {
        return {xyzLimits.x(), xyzLimits.y(), xyzLimits.z()};
    }
    else if constexpr (AXIS == Axis::Y) {
        return {xyzLimits.y(), xyzLimits.x(), xyzLimits.z()};
    }
    else {
        return {xyzLimits.z(), xyzLimits.x(), xyzLimits.y()};
    }
}

/**
 * Transforms slice, u, v coordinates into world space.
 */
template <Axis AXIS>
constexpr Vec3size svxProject(Vec3size suv)
{
    if constexpr (AXIS == Axis::X) {
        return Vec3size(suv[0], suv[1], suv[2]);
    }
    else if constexpr (AXIS == Axis::Y) {
        return Vec3size(suv[1], suv[0], suv[2]);
    }
    else {
        return Vec3size(suv[1], suv[2], suv[0]);
    }
}

constexpr ColorFormat densityFormatOf(const size_t bits)
{
    switch (bits) {
    case 1: return ColorFormat::V1;
    case 8: return ColorFormat::V8;
    default: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

constexpr ColorFormat rgbFormatOf(const size_t bits)
{
    switch (bits) {
    case 1: return ColorFormat::V1;
    case 24: return ColorFormat::RGB24;
    default: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

constexpr ColorFormat formatOf(const Channel &channel)
{
    switch (channel.type) {
    case ChannelType::DENSITY: return densityFormatOf(channel.bits);
    case ChannelType::COLOR_RGB: return rgbFormatOf(channel.bits);
    default: VXIO_DEBUG_ASSERT_UNREACHABLE();
    }
}

template <Axis AXIS>
void writeSvxLayerToImage(Image &image, const VoxelArray &voxels, size_t slice)
{
    const size_t w = image.width();
    const size_t h = image.height();

    for (size_t u = 0; u < w; ++u) {
        for (size_t v = 0; v < h; ++v) {
            Vec3size xyz = svxProject<AXIS>({slice, u, v});
            Color32 rgb = voxels[xyz];
            image.setPixel(u, v, rgb);
        }
    }
}

}  // namespace

std::string Channel::fullFileNameFormat() const
{
    std::string fileName = xmlNameOf(type);
    toLowerCase(fileName);
    fileName += '/';
    fileName += fileNameFormat;
    return fileName;
}

std::string Channel::toXml() const
{
    std::stringstream result;
    result << "<channel ";
    result << "type=\"" << xmlNameOf(type);
    if (usesId(type)) {
        result << "(" << id << ")";
    }
    result << "\" bits=\"" << bits;
    result << "\" slices=\"" << fullFileNameFormat();
    result << "\"/>";

    return result.str();
}

std::string Manifest::toXml() const
{
    std::stringstream result;
    result << "<?xml version=\"1.0\"?>";
    result << "<grid version=\"1.0\" ";
    result << "gridSizeX=\"" << gridSize_.x() << "\" ";
    result << "gridSizeY=\"" << gridSize_.y() << "\" ";
    result << "gridSizeZ=\"" << gridSize_.z() << "\" ";
    result << "voxelSize=\"" << voxelSize_ << "\" ";
    result << "subvoxelBits=\"" << subvoxelBits_ << "\" ";
    result << "slicesOrientation=\"" << nameOf(slicesOrientation_) << "\">";

    if (not channels.empty()) {
        result << "<channels>";
        for (const auto &channel : channels) {
            result << channel.toXml();
        }
        result << "</channels>";
    }
    result << "<materials></materials>";
    result << "<metadata></metadata>";

    result << "</grid>";
    return result.str();
}

bool Manifest::matches(const VoxelArray &voxels) const
{
    Vec3size size = voxels.dimensions();
    return gridSize().x() >= size.x() && gridSize().y() >= size.y() && gridSize().z() >= size.z();
}

template <Axis AXIS>
ResultCode Serializer::writeChannel(miniz_cpp::zip_file &archive, const SimpleVoxels &svx, const Channel &channel)
{
    if (channel.type == ChannelType::CUSTOM || channel.type == ChannelType::MATERIAL) {
        err = {0, "Writing CUSTOM or MATERIAL channels is not supported"};
        return ResultCode::WRITE_ERROR_UNSUPPORTED_FORMAT;
    }

    std::string fileNameFormat = channel.fullFileNameFormat();
    const char *fileNameFormatCstr = fileNameFormat.c_str();

    const VoxelArray &voxels = svx.voxels;
    const SvxLimits lims = svxLimitsOf<AXIS>(voxels.dimensions());

    const ColorFormat format = formatOf(channel);
    /*if (format == QImage::Format_Invalid) {
        err = {0, "Can't determine valid image format for the " + std::string{xmlNameOf(channel.type)} + " channel"};
        return ResultCode::USER_ERROR_INVALID_COLOR_FORMAT;
    }*/

    Image image{lims.u, lims.v, format};
    ByteArrayOutputStream pngStream;
    pngStream.reserve(lims.u * lims.v * bitSizeOf(format) / 8);

    for (size_t slice = 0; slice < lims.slice; ++slice) {
        std::string fileName = voxelio::format(fileNameFormatCstr, slice);
        VXIO_LOG(SPAM, "Writing slice #" + stringify(slice) + ", named" + fileName);

        writeSvxLayerToImage<AXIS>(image, voxels, slice);
        pngStream.clear();
        VXIO_FORWARD_ERROR(png::encode(image, pngStream));
        archive.writestr(fileName, {reinterpret_cast<char *>(pngStream.data()), pngStream.size()});
    }

    return ResultCode::OK;
}

ResultCode Serializer::write(const SimpleVoxels &svx)
{
    miniz_cpp::zip_file archive;
    archive.writestr("manifest.xml", svx.manifest.toXml());
    const Axis axis = svx.manifest.slicesOrientation();

    for (const Channel &channel : svx.manifest.channels) {
        ResultCode code;
        // here we constexpr-ify the axis parameter
        // this allows for compiler optimizations and cleaner code further down the line
        switch (axis) {
        case Axis::X: code = writeChannel<Axis::X>(archive, svx, channel); break;
        case Axis::Y: code = writeChannel<Axis::Y>(archive, svx, channel); break;
        case Axis::Z: code = writeChannel<Axis::Z>(archive, svx, channel); break;
        }
        if (code != ResultCode::OK) {
            return code;
        }
    }

    archive.save([this](const char *data, size_t size) {
        streamWrapper.write(reinterpret_cast<const u8 *>(data), size);
    });
    if (not streamWrapper.good()) {
        err = {streamWrapper.position(), "Stream was not left in a good() state after writing file"};
        return ResultCode::WRITE_ERROR_IO_FAIL;
    }
    return ResultCode::OK;
}

}  // namespace voxelio::svx
