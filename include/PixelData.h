#pragma once
#include "Vector3.h"
#include "Color.h"

struct PixelData {
    Color color;
    double depth;
    Vector3 normal;
    int checks;
    int samples;
};