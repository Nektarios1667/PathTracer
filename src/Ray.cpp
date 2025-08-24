#include "Ray.h"

Ray::Ray() {}
Ray::Ray(const Vector3d& o, const Vector3d& d) : origin(o), direction(d), invDirection(d.inversed()) {}

Vector3d Ray::at(double t) const { return origin + direction * t; }
