#ifndef VXIO_PNG_HPP
#define VXIO_PNG_HPP

#include "voxelio/image.hpp"
#include "voxelio/results.hpp"
#include "voxelio/streamfwd.hpp"

namespace voxelio::png {

/**
 * @brief Encodes an image as a PNG file and writes the result to the output stream.
 * @param image the image
 * @param out the output stream
 * @return the result code
 */
[[nodiscard]] ResultCode encode(const Image &image, OutputStream &out);

[[nodiscard]] std::optional<Image> decode(InputStream &in, usize desiredChannels, std::string &outError);

}  // namespace voxelio::png

#endif  // VXIO_PNG_HPP
