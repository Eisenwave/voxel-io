#include <random>

namespace voxelio::test {

using fast_rng32 = std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0>;
using fast_rng64 = std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, 0>;
using default_rng = std::mt19937;

}  // namespace voxelio::test
