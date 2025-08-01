#include "Plane.h"
#include "Color.h"
#include "Ray.h"
#include "Utilities.h"
#include "AABB.h"
#include <cfloat>
#include <algorithm>

Plane::Plane() : normal(Vector3()) {}
// Note - the "size" parameter is world axis-aligned, and is meant to be a rough estimation for acceleration structures
Plane::Plane(const Vector3& pos, const Vector3& norm, const Vector3& size, const Material& mat) : normal(norm.normalized()), position(pos), size(size) {
    material = mat;
}
bool Plane::intersectsRay(const Ray& ray, float& outT) const {
    // Cull backface
    float denom = normal.dot(ray.direction);
    if (denom >= 0) return false;

    // Get intersection
    float t = (position - ray.origin).dot(normal) / denom;
    if (t < 0) return false;

    outT = t;
    return true;
}

Vector3 Plane::getNormalAt(const Vector3& point) const {
    return normal;
}

AABB Plane::getBoundingBox() const {
    return { position - size * 0.5f, position + size * 0.5f };
}