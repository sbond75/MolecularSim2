#include <RandomNumberGenerator.hpp>

using namespace godot;

namespace rng {
    real_t rand(RandomNumberGenerator& rng);
    size_t randp(RandomNumberGenerator& rng, const std::vector<real_t>& probabilities);
};
