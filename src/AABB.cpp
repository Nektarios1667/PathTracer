#include "AABB.h"
#include "Vector3.h"
#include <algorithm>


bool AABB::rayHit(const Ray& ray, double& tHit) const {
    // X
    double tx1 = (lower.x - ray.origin.x) / (double)ray.direction.x;
    double tx2 = (upper.x - ray.origin.x) / (double)ray.direction.x;
    if (tx1 > tx2) std::swap<double>(tx1, tx2);
    
    // Y
    double ty1 = (lower.y - ray.origin.y) / (double)ray.direction.y;
    double ty2 = (upper.y- ray.origin.y) / (double)ray.direction.y;
    if (ty1 > ty2) std::swap<double>(ty1, ty2);

    // Z
    double tz1 = (lower.z - ray.origin.z) / (double)ray.direction.z;
    double tz2 = (upper.z - ray.origin.z) / (double)ray.direction.z;
    if (tz1 > tz2) std::swap<double>(tz1, tz2);

    double t1 = std::max({tx1, ty1, tz1});
    double t2 = std::min({tx2, ty2, tz2});
    if (t1 <= t2 && t2 >= 0) {
        tHit = std::max(t1, 0.0);
        return true;
    }
    return false;
}

Vector3 AABB::center() const {
    return (lower + upper) * 0.5f;
}