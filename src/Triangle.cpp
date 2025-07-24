#include "Triangle.h"
#include "Color.h"
#include "Ray.h"
#include "Utilities.h"
#include "AABB.h"
#include <algorithm>

Triangle::Triangle() : v0(0), v1(0), v2(0) {
    this->material = {Color(), Color()};
    normal = Vector3();
}
Triangle::Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, Material material) : v0(v0), v1(v1), v2(v2) {
    Hittable::material = material;
    normal = (v1 - v0).cross(v2 - v0).normalized();
}
bool Triangle::isPointInside(const Vector3& point) const {
    return false; // TODO
}
bool Triangle::intersectsRay(const Ray& ray, float& outT) const {
    // Edges
    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;

    // Determinant
    Vector3 h = ray.direction.cross(edge2);
    float a = edge1.dot(h);

    // Cull backface
    if (a < Utilities::EPSILON) return false;

    // U barymetric
    Vector3 s = ray.origin - v0;
    float u = s.dot(h) / a;
    if (u < 0 || u > 1) return false;

    // V baymetric
    Vector3 q = s.cross(edge1);
    float v = ray.direction.dot(q) / a;
    if (v < 0 || u + v > 1) return false;

    // Get intersection distance
    float t = edge2.dot(q) / a;
    if (t > Utilities::EPSILON) {
        outT = t;
        return true; // Hit
    }

    return false; // Ray doesn't intersect
}
Vector3 Triangle::getNormalAt(const Vector3& point) const {
    return normal;
}

AABB Triangle::getBoundingBox() const {
    return { Vector3(
        min({ v0.x, v1.x, v2.x }),
        min({ v0.y, v1.y, v2.y }),
        min({ v0.z, v1.z, v2.z })
    ),
        Vector3(
        max({ v0.x, v1.x, v2.x }),
        max({ v0.y, v1.y, v2.y }),
        max({ v0.z, v1.z, v2.z })
    ),
    };
}