#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "AABB.h"
#include "memory"

struct AABB {
    Vector3 lower;
    Vector3 upper;

    bool rayHit(const Ray& ray) const;
    Vector3 center() const;
};