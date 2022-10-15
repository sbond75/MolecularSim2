#include <Vector3.hpp>
#include <Defs.hpp> // real_t
#include <vector>

//[option:vcl]//
#define VCL_NAMESPACE vcl
#include <version2/vectorclass.h> // https://github.com/vectorclass/version2 -- Agner Fog's SIMD vector library
// //

using namespace godot;

namespace sim {

  //[option:vcl]//
  using Vector3 = vcl::Vec4f;
  static inline ::Vector3 vec3ToGodotVec3(Vector3 simVec3) {
    real_t retval[3];
    simVec3.store_partial(3, retval);
    return *reinterpret_cast<::Vector3*>(retval);
  }
  // //
  //[non-option:vcl]//
  // static inline ::Vector3 vec3ToGodotVec3(Vector3 simVec3) {
  //   return simVec3;
  // }
  // //

  enum MoleculeType {
    Nitrogen,
    Oxygen,
    Argon,
    CarbonDioxide,
    
    GenericLightweightMolecule
  };

  extern real_t mass(MoleculeType type);
  
  struct ForceInfo {
    Vector3 acceleration;

    void applyForce(Vector3 f);
  };

  struct Molecule {
    MoleculeType type;
    Vector3 pos;
    Vector3 velocity;

    //[deprecated:badImpl] void applyForce(Vector3 f, float deltaTime);
    void updatePos(float deltaTime);

    void finalizeForces(ForceInfo& forceInfo, float deltaTime);
  };

  struct Wall {
    Vector3 pos;
  };

  extern Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2, real_t& out_forceMagnitude, float& out_distSquared, real_t& out_dist);
  extern real_t potentialEnergyOnMolecule(real_t sigma, real_t epsilon, real_t dist);
  
  extern void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Molecule>& molecules, std::vector<ForceInfo>& moleculeForces, std::vector<Wall>& walls, ::Vector3 boundingBoxWalls[2] /*top-left-back x,y,z and then bottom-right-front x,y,z*/, real_t& out_uSum /*potential energy sum*/, real_t& out_virSum /*? is the sum of {forces applied times the radius squared}.*/);

  // Populates the given `out_` values with what the simulation indicates they are.
  extern void evaluateProperties(std::vector<Molecule>& molecules,::Vector3 boundingBoxWalls[2] /*top-left-back x,y,z and then bottom-right-front x,y,z*/, real_t uSum, real_t virSum, real_t& out_kineticEnergy, real_t& out_totalEnergy, real_t& out_pressure, real_t& out_temperature, real_t& out_density);
}
