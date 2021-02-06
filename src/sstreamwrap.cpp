#include "voxelio/sstreamwrap.hpp"

#include <sstream>

namespace voxelio::detail {

std::stringstream *stringstream_make()
{
    return new std::stringstream;
}

std::stringstream *stringstream_make(std::string contents)
{
    return new std::stringstream{contents};
}

std::istream *stringstream_to_istream(std::stringstream *stream)
{
    return static_cast<std::istream *>(stream);
}

std::ostream *stringstream_to_ostream(std::stringstream *stream)
{
    return static_cast<std::ostream *>(stream);
}

void stringstream_precision(std::stringstream *stream, std::streamsize precision)
{
    stream->precision(precision);
}

void stringstream_free(std::stringstream *stream)
{
    delete stream;
}

bool stringstream_fail(std::stringstream *stream)
{
    return stream->fail();
}

std::string stringstream_to_string(std::stringstream *stream)
{
    return stream->str();
}

}  // namespace voxelio::detail
