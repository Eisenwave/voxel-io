#ifndef VXIO_UTIL_PRIVATE_HPP
#define VXIO_UTIL_PRIVATE_HPP

#include "results.hpp"

#define VXIO_FORWARD_ERROR(expr)                                                                          \
    if (auto exprResult__ = expr; ::voxelio::isError(static_cast<::voxelio::ResultCode>(exprResult__))) { \
        return exprResult__;                                                                              \
    }

#define VXIO_NO_EOF()                              \
    if constexpr (::voxelio::build::DEBUG)         \
        VXIO_DEBUG_ASSERT(not this->stream.eof()); \
    else if (this->stream.eof())                   \
    return ::voxelio::ReadResult::unexpectedEof(stream.position())

#endif  // VXIO_UTIL_PRIVATE_HPP
