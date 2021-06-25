#include "voxelio/sstreamwrap.hpp"

#include <sstream>

namespace voxelio::detail {

static_assert(std::is_same_v<signedSize, std::streamsize>);

std::istringstream *istringstream_make()
{
    return new std::istringstream;
}

std::ostringstream *ostringstream_make()
{
    return new std::ostringstream;
}

std::istringstream *istringstream_make(std::string contents)
{
    return new std::istringstream{contents};
}

std::istream *stringstream_to_istream(std::stringstream *stream)
{
    return static_cast<std::istream *>(stream);
}

std::istream *istringstream_to_istream(std::istringstream *stream)
{
    return static_cast<std::istream *>(stream);
}

std::ostream *ostringstream_to_ostream(std::ostringstream *stream)
{
    return static_cast<std::ostream *>(stream);
}

void ostream_precision(std::ostream *stream, std::streamsize precision)
{
    stream->precision(precision);
}

bool ostream_fail(std::ostream *stream)
{
    return stream->fail();
}

void ostream_free(std::ostream *stream)
{
    delete stream;
}

bool istream_fail(std::istream *stream)
{
    return stream->fail();
}

void istream_free(std::istream *stream)
{
    delete stream;
}

std::string ostringstream_to_string(std::ostringstream *stream)
{
    return stream->str();
}

}  // namespace voxelio::detail
