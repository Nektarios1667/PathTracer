#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"
#include "AABB.h"
#include <tuple>
#include <vector>

struct Sphere : public Hittable {
    float radius;
    float radiusSquared;
    Vector3 center;

    Sphere();
    Sphere(const Vector3& center, float radius, shared_ptr<Material> mat);

    bool intersectsRay(const Ray& ray, double& outT) const override;
    Vector3d getNormalAt(const Vector3d& point, const Vector3d& dir) const override;
    AABB getBoundingBox() const override;
    //std::tuple<float, float> getUV(const Vector3& point) const;
    //Color getUVColor(const Vector3& point) const;
};