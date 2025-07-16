#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Color.h"

struct Material {
    Color albedo;
    Color emission;
    float reflectivity;
    float roughness;
};

class Hittable {
    public:
        Vector3 center;
        Material material;

        virtual bool isPointInside(const Vector3& point) const = 0;
        virtual bool intersectsRay(const Ray& ray, float& outT) const = 0;
        virtual bool canIntersect(const Ray& ray) const = 0;
        virtual Vector3 getNormalAt(const Vector3& point) const = 0;

        virtual ~Hittable() = default;
};
