#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Ray.h"
#include "Hittable.h"
#include "AABB.h"

// Note - the "size" parameter is world axis-aligned, and is meant to be a rough estimation for acceleration structures
struct Plane : public Hittable {
public:
    Vector3 normal;
    Vector3 position;
    Vector3 size;

    Plane();
    Plane(const Vector3& pos, const Vector3& norm, const Vector3& size, const Material& mat);

    bool intersectsRay(const Ray& ray, float& outT) const override;
    Vector3 getNormalAt(const Vector3& point) const override;
    AABB getBoundingBox() const override;
};