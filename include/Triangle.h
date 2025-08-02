#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"
#include "AABB.h"

struct Triangle : public Hittable {
public:
    Vector3 v0, v1, v2;
    Vector3 normal;

    Triangle();
    Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, shared_ptr<Material> mat);

    bool intersectsRay(const Ray& ray, float& outT) const override;
    Vector3 getNormalAt(const Vector3& point) const override;
    AABB getBoundingBox() const override;
};