#include "voxelio/voxelio.hpp"

#include "voxelio/log.hpp"
#include "voxelio/stringify.hpp"

#include <sstream>

namespace voxelio {

AbstractReader::AbstractReader(InputStream &stream, u64 dataLength) noexcept : stream{stream}, dataLength{dataLength} {}

float AbstractReader::progress() noexcept
{
    if (dataLength == DATA_LENGTH_UNKNOWN) {
        return std::numeric_limits<float>::signaling_NaN();
    }
    return static_cast<float>(stream.position() + 1) / float(dataLength);
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

}  // namespace voxelio
