#ifndef VXIO_SSTREAMWRAP_HPP
#define VXIO_SSTREAMWRAP_HPP

#include <iosfwd>
#include <string>

namespace voxelio::detail {

std::stringstream *stringstream_make();
std::stringstream *stringstream_make(std::string contents);

std::istream *stringstream_to_istream(std::stringstream *);
std::istream *stringstream_to_ostream(std::stringstream *);

bool stringstream_fail(std::stringstream *);
void stringstream_precision(std::stringstream *stream, std::streamsize precision);
std::string stringstream_to_string(std::stringstream *stream);

void stringstream_free(std::stringstream *);

}  // namespace voxelio::detail

#endif  // SSTREAMWRAP_HPP
