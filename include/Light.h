#pragma once
#include "Vector3.h"
#include "Color.h"

struct Light {
    Vector3 position;
    Color color;
    float intensity;
};
