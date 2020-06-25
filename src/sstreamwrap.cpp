#include "sstreamwrap.hpp"

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
