#ifndef VXIO_LODEPNGFWD_HPP
#define VXIO_LODEPNGFWD_HPP

#include "../streamfwd.hpp"
#include "../types.hpp"

typedef enum LodePNGColorType {
  LCT_GREY = 0, /*grayscale: 1,2,4,8,16 bit*/
  LCT_RGB = 2, /*RGB: 8,16 bit*/
  LCT_PALETTE = 3, /*palette: 1,2,4,8 bit*/
  LCT_GREY_ALPHA = 4, /*grayscale with alpha: 8,16 bit*/
  LCT_RGBA = 6, /*RGB with alpha: 8,16 bit*/
  /*LCT_MAX_OCTET_VALUE lets the compiler allow this enum to represent any invalid
  byte value from 0 to 255 that could be present in an invalid PNG file header. Do
  not use, compare with or set the name LCT_MAX_OCTET_VALUE, instead either use
  the valid color type names above, or numeric values like 1 or 7 when checking for
  particular disallowed color type byte values, or cast to integer to print it.*/
  LCT_MAX_OCTET_VALUE = 255
} LodePNGColorType;

const char* lodepng_error_text(unsigned code);

namespace lodepng {

class State;

unsigned encode(voxelio::OutputStream& out, const voxelio::u8 in[], unsigned w, unsigned h,
                LodePNGColorType colortype, unsigned bitdepth);

unsigned encode(voxelio::OutputStream& out,
                const voxelio::u8 in[], unsigned w, unsigned h,
                State& state);

}

#endif // VXIO_LODEPNGFWD_HPP
