#pragma once
#include "Vector3.h"

class Ray {
    public:
        Vector3 origin;
        Vector3 direction;
    
        Ray();
        Ray(const Vector3& o, const Vector3& d);
    
        Vector3 at(float t) const;
    };