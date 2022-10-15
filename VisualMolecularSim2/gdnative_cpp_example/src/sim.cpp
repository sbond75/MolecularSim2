#include "sim.hpp"

#include <cmath>
#include <string>

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

  // sigma: length scale
  // epsilon: governs the strength of the interaction
  // p1: position of molecule 1 (vector (bold) "r_i")
  // p2: position of molecule 2 (vector "r_j")
  Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2) {
    // r_c: maximum distance
    real_t r_c = pow(2, 1.0/6) * sigma;

//#define pow fs_power
//#define pow ipow
    //[option:vcl]//
    real_t dist = sqrt(vcl::horizontal_add(vcl::square(p1 - p2))); // "r" aka "r_ij"
    //
    //[non-option:vcl]//
    // real_t dist = p1.distance_to(p2); // "r" aka "r_ij"
    // //
    if (dist >= r_c) {
      // Maximum distance reached
      return Vector3(0, 0, 0, 0); //Vector3::ZERO;
    }
    return 48 * epsilon / (sigma * sigma) * (pow(sigma / dist, 14) - 0.5 * pow(sigma / dist, 8)) * (p1 - p2);
  }
#undef pow

  void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Molecule>& molecules, std::vector<ForceInfo>& moleculeForces, std::vector<Wall>& walls, ::Vector3 boundingBoxWalls[2]) {
    for (size_t i = 0; i < molecules.size(); i++) {
      Molecule& m1 = molecules[i];
      ForceInfo& f1 = moleculeForces[i]; //[non-deprecated:badImpl]
      for (size_t j = 0; j < molecules.size(); j++) {
	if (i == j) continue;
	Molecule& m2 = molecules[j];
	ForceInfo& f2 = moleculeForces[j]; //[non-deprecated:badImpl]
	
	Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos);
	//[deprecated:badImpl] m1.applyForce(force, deltaTime);
        f1.applyForce(force);
      }

      for (size_t j = 0; j < walls.size(); j++) {
	Wall& m2 = walls[j];

	Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos);
	//[deprecated:badImpl] m1.applyForce(force, deltaTime);
        f1.applyForce(force);
      }

      //[deprecated:badImpl] m1.updatePos(deltaTime);
    }

    //[non-deprecated:badImpl]//
    for (size_t i = 0; i < molecules.size(); i++) {
      Molecule& m1 = molecules[i];
      ForceInfo& f1 = moleculeForces[i];
      m1.finalizeForces(f1, deltaTime);
      m1.updatePos(deltaTime);

      // Bounce on walls
      if (m1.pos[0] < boundingBoxWalls[0][0] || m1.pos[0] > boundingBoxWalls[1][0]) {
        m1.velocity.insert(0, -m1.velocity[0]);
      }
      if (m1.pos[1] < boundingBoxWalls[0][1] || m1.pos[1] > boundingBoxWalls[1][1]) {
        m1.velocity.insert(1, -m1.velocity[1]);
      }
      if (m1.pos[2] < boundingBoxWalls[0][2] || m1.pos[2] > boundingBoxWalls[1][2]) {
        m1.velocity.insert(2, -m1.velocity[2]);
      }
    }
    // //
  }
}
