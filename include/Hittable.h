#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Color.h"
#include "AABB.h"

struct Material {
    Color albedo;
    Color emission;
    float reflectivity;
    float roughness;
};

class Hittable {
    public:
        Material material;

        virtual bool isPointInside(const Vector3& point) const = 0;
        virtual bool intersectsRay(const Ray& ray, float& outT) const = 0;
        virtual Vector3 getNormalAt(const Vector3& point) const = 0;
        virtual AABB getBoundingBox() const = 0;

        virtual ~Hittable() = default;
};
