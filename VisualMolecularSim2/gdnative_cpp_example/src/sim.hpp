#include <Vector3.hpp>
#include <Defs.hpp> // real_t
#include <vector>

#define VCL_NAMESPACE vcl
#include <version2/vectorclass.h> // https://github.com/vectorclass/version2 -- Agner Fog's SIMD vector library

using namespace godot;

namespace sim {

  using Vector3 = vcl::Vec4f;
  static inline ::Vector3 vec3ToGodotVec3(Vector3 simVec3) {
    real_t retval[3];
    simVec3.store_partial(3, retval);
    return *reinterpret_cast<::Vector3*>(retval);
  }

  enum MoleculeType {
    Nitrogen,
    Oxygen,
    Argon,
    CarbonDioxide,
    
    GenericLightweightMolecule
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
