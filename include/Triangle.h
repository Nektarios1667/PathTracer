#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"
#include "AABB.h"

struct Triangle : public Hittable {
public:
    Vector3 v0, v1, v2;
    Vector3d normal;

    Triangle();
    Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, shared_ptr<Material> mat);

    bool intersectsRay(const Ray& ray, double& outT) const override;
    Vector3d getNormalAt(const Vector3d& point, const Vector3d& dir) const override;
    AABB getBoundingBox() const override;
};