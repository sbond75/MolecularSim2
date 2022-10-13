#include <Vector3.hpp>
#include <vector>

using namespace godot;

namespace sim {

  struct Argon {
    Vector3 pos;
    Vector3 velocity;
    static real_t mass;
  };

  extern Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2);
  
  extern void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Argon>& molecules);

}
