#include "sim.hpp"

#include <cmath>

// BROKEN?/slow?:
// extern "C" double fs_power(double x, long n);

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

  void Molecule::updatePos(float deltaTime) {
    pos += velocity * deltaTime;
  }

  void Molecule::finalizeForces(ForceInfo& forceInfo, float deltaTime) {
    velocity += forceInfo.acceleration / mass(type) * deltaTime;
    forceInfo.acceleration = {0};
  }

  // sigma: length scale
  // epsilon: governs the strength of the interaction
  // p1: position of molecule 1 (vector (bold) "r_i")
  // p2: position of molecule 2 (vector "r_j")
  Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2) {
    // r_c: maximum distance
    real_t r_c = pow(2, 1.0/6) * sigma;

//#define pow fs_power
#define pow ipow
    real_t dist = sqrt(vcl::horizontal_add(vcl::square(p1 - p2))); //p1.distance_to(p2); // "r" aka "r_ij"
    if (dist >= r_c) {
      // Maximum distance reached
      return Vector3(0, 0, 0, 0); //Vector3::ZERO;
    }
    return 48 * epsilon / (sigma * sigma) * (pow(sigma / dist, 14) - 0.5 * pow(sigma / dist, 8)) * (p1 - p2);
  }
#undef pow

  void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Molecule>& molecules, std::vector<ForceInfo>& moleculeForces, std::vector<Wall>& walls) {
    for (size_t i = 0; i < molecules.size(); i++) {
      Molecule& m1 = molecules[i];
      ForceInfo& f1 = moleculeForces[i];
      for (size_t j = 0; j < molecules.size(); j++) {
	if (i == j) continue;
	Molecule& m2 = molecules[j];
	ForceInfo& f2 = moleculeForces[j];
	
	Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos);
	f1.applyForce(force);
      }

      for (size_t j = 0; j < walls.size(); j++) {
	Wall& m2 = walls[j];

	Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos);
	f1.applyForce(force);
      }
    }

    for (size_t i = 0; i < molecules.size(); i++) {
      Molecule& m1 = molecules[i];
      ForceInfo& f1 = moleculeForces[i];
      //m1.finalizeForces(f1, deltaTime);
      m1.updatePos(deltaTime);
    }
  }
}
