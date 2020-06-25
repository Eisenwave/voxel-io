#include "voxelio.hpp"

#include "log.hpp"
#include "stringify.hpp"

#include <sstream>

namespace voxelio {

AbstractReader::AbstractReader(InputStream &stream, u64 dataLength) noexcept : stream{stream}, dataLength{dataLength} {}

float AbstractReader::progress() noexcept
{
    if (dataLength == DATA_LENGTH_UNKNOWN) {
        return std::numeric_limits<float>::signaling_NaN();
    }
    return static_cast<float>(stream.position() + 1) / dataLength;
}

AbstractListWriter::AbstractListWriter(OutputStream &stream) noexcept : stream{stream} {}

std::string informativeNameOf(ResultCode code)
{
    std::stringstream stream;
    stream << "0x";
    stream << voxelio::stringifyHex(static_cast<unsigned>(code));
    stream << " (";
    stream << nameOf(code);
    stream << ')';
    return stream.str();
}

Vec3u32 AbstractListWriter::getCanvasDimensions() const
{
    return canvasDims.value();
}

bool AbstractListWriter::setCanvasDimensions(Vec3u32 dims)
{
    if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0) {
        VXIO_LOG(WARNING, "zero canvas dimensions");
        return false;
    }
    VXIO_LOG(SPAM, "canvas dimensions for writer set to " + dims.toString());
    canvasDims = dims;
    return true;
}

}  // namespace voxelio
