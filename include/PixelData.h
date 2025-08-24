#pragma once
#include "Vector3.h"
#include "Color.h"
#include "Vector3d.h"

struct PixelData {
    Color color;
    double depth;
    Vector3d normal;
    int checks;
    int samples;
};