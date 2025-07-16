#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"

struct Sphere : public Hittable {
public:
    float radius;
    float radiusSquared;

    Sphere();
    Sphere(const Vector3& center, float radius, Material material);

    bool isPointInside(const Vector3& point) const override;
    bool intersectsRay(const Ray& ray, float& outT) const override;
    bool canIntersect(const Ray& ray) const override;
    Vector3 getNormalAt(const Vector3& point) const override;
};