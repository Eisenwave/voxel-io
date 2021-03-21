#ifndef VXIO_TEST_RANDOM_HPP
#define VXIO_TEST_RANDOM_HPP

#include <random>

namespace voxelio::test {

using fast_rng32 = std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0>;
using fast_rng64 = std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, 0>;
using default_rng = std::mt19937;

inline std::uint32_t hardwareSeed()
{
    return std::random_device{}();
}

}  // namespace voxelio::test

#endif
