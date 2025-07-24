#include "Ray.h"

Ray::Ray() {}
Ray::Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d), invDirection(d.inversed()) {}

Vector3 Ray::at(float t) const { return origin + direction * t; }
