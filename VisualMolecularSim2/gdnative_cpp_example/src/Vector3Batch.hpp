//[option:vcl]//
#define VCL_NAMESPACE vcl
#include <version2/vectorclass.h> // https://github.com/vectorclass/version2 -- Agner Fog's SIMD vector library
// //

struct Vector3Batch {
  vcl::Vec16f x; // Each component of this vector is the x-coordinate of a "logical" vector, with up to 16 logical vectors' x-coordinates stored here.
  vcl::Vec16f y; // Same as the above, but for `y`.
  vcl::Vec16f z; // Same as the above, but for `z`.
};
static inline Vector3Batch operator-(const Vector3Batch& v1, const Vector3Batch& v2) {
  Vector3Batch retval;
  retval.x = v1.x - v2.x;
  retval.y = v1.y - v2.y;
  retval.z = v1.z - v2.z;
  return retval;
}

namespace v3batch {
  static inline Vector3Batch square(const Vector3Batch& v) {
   Vector3Batch retval;
   retval.x = vcl::square(v.x);
   retval.y = vcl::square(v.y);
   retval.z = vcl::square(v.z);
   return retval;
  }
  static inline Vector3Batch horizontal_add(const Vector3Batch& v) {
   Vector3Batch retval;
   retval.x = vcl::square(v.x);
   retval.y = vcl::square(v.y);
   retval.z = vcl::square(v.z);
   return retval;
  }
}

- NEXT: pv=nrt abstractions for computing pressure etc. in hvac
- gravity
- low priority: data layout for better simd

