#include "sim.hpp"

#include <cmath>
#include <string>
#include "memoization.hpp"

// BROKEN?/slow?:
// extern "C" double fs_power(double x, long n);

extern "C" float sqrt14(float x);

// https://www.agner.org/optimize/optimizing_assembly.pdf
// Example 6.1a. Raise double x to the power of int n.
double ipow (double x, int n) {
 unsigned int nn = abs(n); // absolute value of n
 double y = 1.0; // used for multiplication
 while (nn != 0) { // loop for each bit in nn
   if (nn & 1) y *= x; // multiply if bit = 1
   x *= x; // square x
   nn >>= 1; // get next bit of nn
 }
 if (n < 0) y = 1.0 / y; // reciprocal if n is negative
 return y; // return y = pow(x,n)
}

// NOTE: this function sometimes hangs forever:
// // Example 6.1f. Gnu-style inline assembly, Intel syntax
// double ipow (double x, int n) {
//  double y;
//  __asm__ (
//  ".intel_syntax noprefix \n" // use Intel syntax for convenience
//  "cdq \n"
//  "xor eax, edx \n"
//  "sub eax, edx \n"
//  "fld1 \n"
//  "jz 9f \n"
//  ".att_syntax prefix \n" // AT&T syntax needed for %[xx]
//  "fldl %[xx] \n" // memory operand substituted with x
//  ".intel_syntax noprefix \n" // switch to Intel syntax again
//  "jmp 2f \n"
//  "1: \n"
//  "fmul st(0), st(0) \n"
//  "2: \n"
//  "shr eax, 1 \n"
//  "jnc 1b \n"
//  "fmul st(1), st(0) \n"
//  "jnz 1b \n"
//  "fstp st(0) \n"
//  "test edx, edx \n"
//  "jns 9f \n"
//  "fld1 \n"
//  "fdivrp \n"
//  "9: \n"
//  ".att_syntax prefix \n" // switch back to AT&T syntax
//  // output operands:
//  : "=t" (y) // output in top-of-stack goes to y
//  // input operands:
//  : [xx] "m" (x), "a" (n) // input memory %[x] for x, eax for n
//  // clobbered registers:
//  : "%edx", "%st(1)" ); // edx and st(1) are modified
//  return y;
// }

namespace sim {

  real_t mass(MoleculeType type) {
    switch (type) {
    case Nitrogen:
      return 14.0067;
    case Oxygen:
      return 15.999;
    case Argon:
      return 39.948; // Daltons ( https://en.wikipedia.org/wiki/Atomic_mass ). To convert to kg, multiply Daltons by 1.66053892173E-27 ( http://www.conversion-website.com/mass/dalton-to-kilogram.html )
    case CarbonDioxide:
      return 44.009;
    case GenericLightweightMolecule:
      return 3.9;
    default:
      exit(1);
    }
  }

  void ForceInfo::applyForce(Vector3 f) {
    acceleration += f;
  }

  //[deprecated:badImpl]
  // void Molecule::applyForce(Vector3 f, float deltaTime) {
  //   velocity += f / mass(type) * deltaTime;
  // }

  void Molecule::updatePos(float deltaTime) {
    Vector3 temp = velocity * deltaTime;
    Vector3 temp2;
    temp2.load_partial(3, reinterpret_cast<float*>(&temp));
    pos += temp2;
  }

  void Molecule::finalizeForces(ForceInfo& forceInfo, float deltaTime) {
    // std::string s = std::to_string(forceInfo.acceleration[0]) + " " + std::to_string(forceInfo.acceleration[1]) + " " + std::to_string(forceInfo.acceleration[2]);
    // Godot::print(s.c_str());
    velocity += forceInfo.acceleration / mass(type) * deltaTime;
    forceInfo.acceleration = {0};
  }

  // https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction
  // float hsum_ps_sse3(__m128 v) {
  //   __m128 shuf = _mm_movehdup_ps(v);        // broadcast elements 3,1 to 2,0
  //   __m128 sums = _mm_add_ps(v, shuf);
  //   shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
  //   sums        = _mm_add_ss(sums, shuf);
  //   return        _mm_cvtss_f32(sums);
  // }
  
  // sigma: length scale
  // epsilon: governs the strength of the interaction
  // p1: position of molecule 1 (vector (bold) "r_i")
  // p2: position of molecule 2 (vector "r_j")
  Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2, real_t& out_forceMagnitude, float& out_distSquared) {
    // r_c: maximum distance
    real_t r_c = pow(2, 1.0/6) * sigma;

//#define pow fs_power
#define pow ipow
//#define sqrt memoization::sqrt_memoize
//#define sqrt sqrt14
    Vector3 r = p1 - p2;
    //[option:vcl]//
    Vector3 squared = vcl::square(r);
    //out_distSquared = hsum_ps_sse3(*reinterpret_cast<__m128*>(&squared));
    out_distSquared = vcl::horizontal_add(squared);
    //out_dist = sqrt(out_distSquared); // "r" aka "r_ij"
    //
    //[non-option:vcl]//
    // real_t dist = p1.distance_to(p2); // "r" aka "r_ij"
    // //
    if (out_distSquared >= r_c * r_c) {
      // Maximum distance reached
      out_forceMagnitude = 0;
      return Vector3(0, 0, 0, 0); //Vector3::ZERO;
    }
    static double cached1 = 48 * epsilon / (sigma * sigma);
    out_forceMagnitude = cached1 * (pow(sigma, 14) / pow(out_distSquared, 7)) - 0.5 * (pow(sigma, 8) / pow(out_distSquared, 4));
    return out_forceMagnitude * r;
  }
#undef pow
  
  // sigma: length scale. units: Å
  // epsilon: governs the strength of the interaction. units: eV
  // p1: position of molecule 1 (vector (bold) "r_i"). units: Å
  // p2: position of molecule 2 (vector "r_j"). units: Å
  // Returns units: eV
  real_t potentialEnergyOnMolecule(real_t sigma, real_t epsilon, real_t distSquared) {
    // r_c: maximum distance
    real_t r_c = pow(2, 1.0/6) * sigma;

//#define pow fs_power
#define pow ipow
    if (distSquared >= r_c * r_c) {
      // Maximum distance reached
      return 0;
    }
    return 4 * epsilon * (pow(sigma, 12) / pow(distSquared, 6) - pow(sigma, 6) / pow(distSquared, 3)) + epsilon;
  }
#undef pow

  void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Molecule>& molecules, std::vector<ForceInfo>& moleculeForces, std::vector<Wall>& walls, ::Vector3 boundingBoxWalls[2], real_t& out_uSum, real_t& out_virSum) {
    out_uSum = 0;
    out_virSum = 0;
    real_t fcVal;
    float distSquared;
    real_t dist;
    size_t nMol = molecules.size();
    if (nMol > 0) {
      for (size_t i = 0; i < nMol - 1; i++) {
        Molecule& m1 = molecules[i];
        ForceInfo& f1 = moleculeForces[i]; //[non-deprecated:badImpl]
        for (size_t j = i + 1; j < nMol; j++) {
          Molecule& m2 = molecules[j];
          ForceInfo& f2 = moleculeForces[j]; //[non-deprecated:badImpl]

          Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos, fcVal, distSquared);
          out_uSum += potentialEnergyOnMolecule(sigma, epsilon, distSquared);
          out_virSum += fcVal * distSquared;
          //[deprecated:badImpl] m1.applyForce(force, deltaTime);
          f1.applyForce(force);
          f2.applyForce(-force); // https://pythoninchemistry.org/sim_and_scat/molecular_dynamics/build_an_md
        }

        size_t nWall = walls.size();
        for (size_t j = 0; j < nWall; j++) {
          Wall& m2 = walls[j];

          Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos, fcVal, distSquared);
          // TODO: change out_uSum here?
          // TODO: change out_virSum here?
          //[deprecated:badImpl] m1.applyForce(force, deltaTime);
          f1.applyForce(force);
        }

        //[deprecated:badImpl] m1.updatePos(deltaTime);
      }
    }
  
    //[non-deprecated:badImpl]//
    for (size_t i = 0; i < nMol; i++) {
      Molecule& m1 = molecules[i];
      ForceInfo& f1 = moleculeForces[i];
      m1.finalizeForces(f1, deltaTime);
      m1.updatePos(deltaTime);

      // Bounce on walls
      // if (m1.pos[0] < boundingBoxWalls[0][0] || m1.pos[0] > boundingBoxWalls[1][0]) {
      //   m1.velocity.insert(0, -m1.velocity[0]);
      // }
      // if (m1.pos[1] < boundingBoxWalls[0][1] || m1.pos[1] > boundingBoxWalls[1][1]) {
      //   m1.velocity.insert(1, -m1.velocity[1]);
      // }
      // if (m1.pos[2] < boundingBoxWalls[0][2] || m1.pos[2] > boundingBoxWalls[1][2]) {
      //   m1.velocity.insert(2, -m1.velocity[2]);
      // }
    }
    // //
  }

  void evaluateProperties(std::vector<Molecule>& molecules, ::Vector3 boundingBoxWalls[2], real_t uSum, real_t virSum, real_t& out_kineticEnergy, real_t& out_totalEnergy, real_t& out_pressure, real_t& out_temperature, real_t& out_density) {
    // Based on https://gist.github.com/lucazammataro/42c4423f2082306824f4dd3352dde807#file-rap_2_measurement_functions-py from the code block labelled "Code 10" on https://towardsdatascience.com/the-lennard-jones-potential-35b2bae9446c
    Vector3 vSum{0};
    real_t vvSum = 0.;
    size_t nMol = molecules.size();
    real_t density = 0;
    for (size_t i = 0; i < nMol; i++) {
      Molecule& m1 = molecules[i];
      vSum += m1.velocity;
      float vv = vcl::horizontal_add(vcl::square(m1.velocity));
      vvSum += vv;

      density += mass(m1.type);
    }
    // Compute length times width times height of `boundingBoxWalls`:
    real_t lwh = abs((boundingBoxWalls[0][0] - boundingBoxWalls[1][0]) * (boundingBoxWalls[0][1] - boundingBoxWalls[1][1]) * (boundingBoxWalls[0][2] - boundingBoxWalls[1][2]));
    density /= lwh; // "density = mass / volume" is done here.
    out_density = density;

    out_kineticEnergy = (0.5 * vvSum) / nMol;
    out_totalEnergy = out_kineticEnergy + (uSum / nMol);
    const size_t NDIM = 3; // Number of dimensions in the simulation
    out_pressure = density * (vvSum + virSum) / (nMol * NDIM);
    out_temperature = vvSum / (nMol * NDIM); // based on page 30 of the art of molecular dynamics simulation pdf
  }
}
