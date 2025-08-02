#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"
#include "AABB.h"

struct Sphere : public Hittable {
public:
    float radius;
    float radiusSquared;
    Vector3 center;

    Sphere();
    Sphere(const Vector3& center, float radius, shared_ptr<Material> mat);

    bool intersectsRay(const Ray& ray, float& outT) const override;
    Vector3 getNormalAt(const Vector3& point) const override;
    AABB getBoundingBox() const override;
};