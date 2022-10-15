#include "string.hpp"

#include <strstream>

namespace string {
  
  godot::String doubleToGodotString(double d) {
    std::strstream ss;
    ss.precision(12);
    ss.width(18);
    ss << std::left << d;
    // https://en.cppreference.com/w/cpp/io/strstream
    std::ends(ss);
    godot::String retval(ss.str());
    ss.freeze(false);
    return retval;
  }
  
}
