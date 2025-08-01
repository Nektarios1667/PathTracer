#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "AABB.h"
#include "memory"

struct AABB {
    Vector3 lower;
    Vector3 upper;

    bool rayHit(const Ray& ray, float& tHit) const;
    Vector3 center() const;
    static AABB combine(const AABB& box1, const AABB& box2) {
        Vector3 smallCorner(
            std::min(box1.lower.x, box2.lower.x),
            std::min(box1.lower.y, box2.lower.y),
            std::min(box1.lower.z, box2.lower.z)
        );
        Vector3 bigCorner(
            std::max(box1.upper.x, box2.upper.x),
            std::max(box1.upper.y, box2.upper.y),
            std::max(box1.upper.z, box2.upper.z)
        );
        return {smallCorner, bigCorner};
    }
};