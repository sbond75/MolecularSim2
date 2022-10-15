#include <chrono>

namespace timing {

    extern std::chrono::time_point<std::chrono::high_resolution_clock> startTimer();

    // Returns time in milliseconds.
    extern double stopTimer(std::chrono::time_point<std::chrono::high_resolution_clock> t1, const char* title = nullptr);

}
