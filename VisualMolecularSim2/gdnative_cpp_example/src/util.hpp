#include <Vector3.hpp>
#include <Basis.hpp>
#include <Quat.hpp>

using namespace godot;

namespace util {

  inline Vector3 midpoint(Vector3 a, Vector3 b) {
      return {(a.x + b.x) * (real_t)0.5, (a.y + b.y) * (real_t)0.5, (a.z + b.z) * (real_t)0.5};
  }

  Basis makeRotationDir(const Vector3& direction, const Vector3& up = Vector3::UP);

  extern const Vector3 X_AXIS;
  extern const Vector3 Y_AXIS;
  extern const Vector3 Z_AXIS;
  Vector3 orthogonal(Vector3 v);
  Quat get_rotation_between(Vector3 u, Vector3 v);

}
