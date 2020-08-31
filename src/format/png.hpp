#ifndef VXIO_PNG_HPP
#define VXIO_PNG_HPP

#include "../image.hpp"
#include "../results.hpp"
#include "../streamfwd.hpp"

namespace voxelio::png {

/**
 * @brief Encodes an image as a PNG file and writes the result to the output stream.
 * @param image the image
 * @param out the output stream
 * @return the result code
 */
ResultCode encode(const Image &image, OutputStream &out);

}  // namespace voxelio::png

#endif  // VXIO_PNG_HPP
