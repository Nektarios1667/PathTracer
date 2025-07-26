#include "AABB.h"
#include "Vector3.h"
#include <algorithm>


bool AABB::rayHit(const Ray& ray) const {
    // X
    float tx1 = (lower.x - ray.origin.x) / ray.direction.x;
    float tx2 = (upper.x - ray.origin.x) / ray.direction.x;
    if (tx1 > tx2) std::swap<float>(tx1, tx2);
    
    // Y
    float ty1 = (lower.y - ray.origin.y) / ray.direction.y;
    float ty2 = (upper.y- ray.origin.y) / ray.direction.y;
    if (ty1 > ty2) std::swap<float>(ty1, ty2);

    // Z
    float tz1 = (lower.z - ray.origin.z) / ray.direction.z;
    float tz2 = (upper.z - ray.origin.z) / ray.direction.z;
    if (tz1 > tz2) std::swap<float>(tz1, tz2);

    float t1 = std::max({tx1, ty1, tz1});
    float t2 = std::min({tx2, ty2, tz2});
    if (t1 <= t2 && t2 >= 0) return true;
}

Vector3 AABB::center() const {
    return (lower + upper) * 0.5f;
}