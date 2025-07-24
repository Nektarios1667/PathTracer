#pragma once
#include "Vector3.h"
#include "Color.h"

struct PixelData {
    Color color;
    float depth;
    Vector3 normal;
};