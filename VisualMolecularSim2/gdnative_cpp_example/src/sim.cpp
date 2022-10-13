#include "sim.hpp"

namespace sim {

  real_t Argon::mass = 39.948; // Daltons ( https://en.wikipedia.org/wiki/Atomic_mass ). To convert to kg, multiply Daltons by 1.66053892173E-27 ( http://www.conversion-website.com/mass/dalton-to-kilogram.html )

  // sigma: length scale
  // epsilon: governs the strength of the interaction
  // p1: position of molecule 1 (vector (bold) "r_i")
  // p2: position of molecule 2 (vector "r_j")
  Vector3 forceOnMolecule(real_t sigma, real_t epsilon, Vector3 p1, Vector3 p2) {
    // r_c: maximum distance
    real_t r_c = pow(2, 1.0/6) * sigma;

    real_t dist = p1.distance_to(p2); // "r" aka "r_ij"
    if (dist >= r_c) {
      // Maximum distance reached
      return Vector3::ZERO;
    }
    return 48 * epsilon / (sigma * sigma) * (pow(sigma / dist, 14) - 0.5 * pow(sigma / dist, 8)) * (p1 - p2);
  }

  void iterate(real_t sigma, real_t epsilon, float deltaTime, std::vector<Argon>& molecules) {
    for (size_t i = 0; i < molecules.size(); i++) {
      Argon& m1 = molecules[i];
      for (size_t j = 0; j < molecules.size(); j++) {
	if (i == j) continue;
	Argon& m2 = molecules[j];
	
	Vector3 force = forceOnMolecule(sigma, epsilon, m1.pos, m2.pos);
	m1.velocity += force / Argon::mass * deltaTime;
	m1.pos += m1.velocity * deltaTime;
      }
    }
  }
}
