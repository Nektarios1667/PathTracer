#pragma once
#include "Vector3.h"
#include "Vector3d.h"

class Ray {
    public:
        Vector3d origin;
        Vector3d direction;
        Vector3d invDirection;
    
        Ray();
        Ray(const Vector3d& o, const Vector3d& d);
    
        Vector3d at(double t) const;
    };