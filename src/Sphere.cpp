#include "Sphere.h"
#include "Color.h"
#include "Ray.h"
#include "Utilities.h"

Sphere::Sphere() : radius(1), radiusSquared(1), center(Vector3()) {
    material = { Color(), Color(), 0.0f, 1.0f };
}
Sphere::Sphere(const Vector3& center, float radius, const Material& mat) : center(center), radius(radius), radiusSquared(radius*radius) {
    material = mat;
}
bool Sphere::intersectsRay(const Ray& ray, float& outT) const {
    Vector3 oc = center - ray.origin;

    float h = oc.dot(ray.direction);
    float c = oc.lengthSquared() - radiusSquared;
    float discriminant = h*h - c;

    if (discriminant < 0.0f) return false;

    float sqrtD = sqrt(discriminant);
    float t1 = h - sqrtD;
    float t2 = h + sqrtD;

    float t = (t1 >= Utilities::EPSILON) ? t1 : ((t2 >= Utilities::EPSILON) ? t2 : -1);
    if (t < 0.0f) return false;

    outT = t;
    return true;
}
Vector3 Sphere::getNormalAt(const Vector3& point) const {
    return (point - center).normalized();
}

AABB Sphere::getBoundingBox() const {
    return { center - Vector3(radius), center + Vector3(radius) };
}

