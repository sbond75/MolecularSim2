#include "util.hpp"

namespace util {

  // https://stackoverflow.com/questions/18558910/direction-vector-to-rotation-matrix
  Basis makeRotationDir(const Vector3& direction, const Vector3& up) {
      Vector3 xaxis = up.cross(direction);
      xaxis.normalize();

      Vector3 yaxis = direction.cross(xaxis);
      yaxis.normalize();

      Vector3 column1, column2, column3;
      column1.x = xaxis.x;
      column1.y = yaxis.x;
      column1.z = direction.x;

      column2.x = xaxis.y;
      column2.y = yaxis.y;
      column2.z = direction.y;

      column3.x = xaxis.z;
      column3.y = yaxis.z;
      column3.z = direction.z;

      return Basis(column1, column2, column3);
  }

  const Vector3 X_AXIS = Vector3::RIGHT;
  const Vector3 Y_AXIS = Vector3::UP;
  const Vector3 Z_AXIS = Vector3::BACK;
  // https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another/1171995#1171995
  Vector3 orthogonal(Vector3 v)
  {
      float x = abs(v.x);
      float y = abs(v.y);
      float z = abs(v.z);

      Vector3 other = x < y ? (x < z ? X_AXIS : Z_AXIS) : (y < z ? Y_AXIS : Z_AXIS);
      return v.cross(other);
  }
  Quat get_rotation_between(Vector3 u, Vector3 v)
  {
    // It is important that the inputs are of equal length when
    // calculating the half-way vector.
    u.normalize();
    v.normalize();

    // Unfortunately, we have to check for when u == -v, as u + v
    // in this case will be (0, 0, 0), which cannot be normalized.
    if (u == -v)
    {
      // 180 degree rotation around any orthogonal vector
	return Quat(orthogonal(u).normalized(), 0);
    }

    Vector3 half = (u + v).normalized();
    return Quat(u.cross(half), u.dot(half));
  }

}
