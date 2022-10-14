#include "rng.hpp"

#include <vector>
#include <numeric>
#include <limits>

namespace rng {

    real_t rand(RandomNumberGenerator& rng) {
	return rng.randf(); // "Return the next random floating point number in the range [0.0, 1.0)." ( https://docs.python.org/3/library/random.html#random.random <- https://stackoverflow.com/questions/33359740/random-number-between-0-and-1 )
    }

    // rand() with probabilities
    size_t randp(RandomNumberGenerator& rng, const std::vector<real_t>& probabilities) {
	assert(abs(std::accumulate(probabilities.begin(), probabilities.end(), 0.0) - 1.0) <= std::numeric_limits<real_t>::epsilon());

	real_t res = rand(rng);
	size_t i = 0;
	for (auto& x : probabilities) {
	    if (res <= x)
		return i;
	    res -= x;
	    i += 1;
	}
	exit(1);
    }

}
