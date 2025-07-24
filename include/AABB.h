#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "memory"

struct AABB {
    Vector3 lower;
    Vector3 upper;

    bool rayHit(const Ray& ray) const;
    AABB surroundingBox(const AABB& box1, const AABB& box2) const;
    Vector3 center() const;
};