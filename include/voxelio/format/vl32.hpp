#ifndef VXIO_VL32_HPP
#define VXIO_VL32_HPP

#include "voxelio/ioutil.hpp"
#include "voxelio/types.hpp"
#include "voxelio/voxelio.hpp"

namespace voxelio::vl32 {

class Reader : public AbstractReader {
public:
    Reader(InputStream &istream, u64 dataLen = DATA_LENGTH_UNKNOWN) : AbstractReader{istream, dataLen} {}

    [[nodiscard]] ReadResult init() noexcept final;
    [[nodiscard]] ReadResult read(Voxel64 buffer[], size_t bufferLength) noexcept final;
    [[nodiscard]] ReadResult read(Voxel32 buffer[], size_t bufferLength) noexcept;

    [[nodiscard]] ReadResult reset() noexcept;

private:
    template <typename Voxel>
    ReadResult read_impl(Voxel buffer[], size_t bufferLength) noexcept;

    [[nodiscard]] ReadResult readVoxel(Voxel32 &out);
};

class Writer : public AbstractListWriter {
public:
    Writer(OutputStream &ostream) : AbstractListWriter{ostream} {}

    [[nodiscard]] ResultCode init() noexcept final;
    [[nodiscard]] ResultCode write(const Voxel32 buffer[], size_t bufferLength) noexcept final;

private:
    [[nodiscard]] ResultCode writeVoxel(Voxel32 voxel) noexcept;
};

}  // namespace voxelio::vl32

#endif  // VOXELIO_VL32_HPP
