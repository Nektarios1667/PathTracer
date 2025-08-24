#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Color.h"
#include "AABB.h"
#include "Vector3d.h"

struct Material {
    Color albedo;
    Color emission;
    float reflectivity;
    float roughness;
    float refractiveIndex;
    inline bool isDielectric() const {
        return refractiveIndex != 1.0f;
    }

    Material(const Color& albedo, const Color& emission, float reflectivity, float roughness, float refractiveIndex) : albedo(albedo), emission(emission), reflectivity(reflectivity), roughness(roughness), refractiveIndex(refractiveIndex) {}
};

class Hittable {
    public:
        shared_ptr<Material> material;

        virtual bool intersectsRay(const Ray& ray, double& outT) const = 0;
        virtual Vector3d getNormalAt(const Vector3d& point, const Vector3d& dir) const = 0;
        virtual AABB getBoundingBox() const = 0;

        virtual ~Hittable() = default;
};
