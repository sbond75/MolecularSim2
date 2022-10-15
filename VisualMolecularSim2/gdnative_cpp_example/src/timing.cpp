#include "timing.hpp"
#include "Godot.hpp"

using namespace godot;

namespace timing {

    std::chrono::time_point<std::chrono::high_resolution_clock> startTimer() {
	// https://stackoverflow.com/questions/1487695/c-cross-platform-high-resolution-timer
	typedef std::chrono::high_resolution_clock Clock;
	auto t1 = Clock::now();
	return t1;
    }

    double stopTimer(std::chrono::time_point<std::chrono::high_resolution_clock> t1, const char* title) {
	typedef std::chrono::high_resolution_clock Clock;
	auto t2 = Clock::now();
	double ms = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() / 1'000'000.0;
	if (title != nullptr) {
	    Godot::print("{0} took {1} milliseconds", title, ms);
	}
	return ms;
    }

}
