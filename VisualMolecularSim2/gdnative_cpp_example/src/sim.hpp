#include <Vector3.hpp>
#include <vector>

using namespace godot;

namespace sim {

  enum MoleculeType {
    Argon,
  };

  extern real_t mass(MoleculeType type);
  
  struct Molecule {
    MoleculeType type;
    Vector3 pos;
    Vector3 velocity;

    void applyForce(Vector3 f, float deltaTime);
    void updatePos(float deltaTime);
  };

  struct Wall {
    Vector3 pos;
  };

  extern Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2);
  
  extern void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Molecule>& molecules, std::vector<Wall>& walls);

}
