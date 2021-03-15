#ifndef VXIO_SSTREAMWRAP_HPP
#define VXIO_SSTREAMWRAP_HPP

#include <iosfwd>
#include <string>

namespace voxelio::detail {

std::istringstream *istringstream_make();
std::ostringstream *ostringstream_make();
std::istringstream *istringstream_make(std::string);

std::istream *istringstream_to_istream(std::istringstream *);
std::ostream *ostringstream_to_ostream(std::ostringstream *);

bool ostream_fail(std::ostream *);
void ostream_free(std::ostream *);

bool istream_fail(std::istream *);
void istream_free(std::istream *);

void ostream_precision(std::ostream *, std::streamsize);
std::string ostringstream_to_string(std::ostringstream *);

}  // namespace voxelio::detail

#endif  // SSTREAMWRAP_HPP
