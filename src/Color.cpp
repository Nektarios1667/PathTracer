#include "Color.h"
#include "Utilities.h"
#include "Vector3.h"
#include <string>
#include <string>
#include <sstream>

using namespace Utilities;
using namespace std;

Color::Color() : r(0), g(0), b(0) {}
Color::Color(float red, float green, float blue) : r(red), g(green), b(blue) {}
Color::Color(float rgb) : r(rgb), g(rgb), b(rgb) {}

Color Color::operator+(const Color& c) const {
    return Color(r + c.r, g + c.g, b + c.b);
}
Color Color::operator+=(const Color& c) {
    r += c.r;
    g += c.g;
    b += c.b;
    return *this;
}
Color Color::operator-(const Color& c) const {
    return Color(r - c.r, g - c.g, b - c.b);
}
Color Color::operator-=(const Color& c) {
    r -= c.r;
    g -= c.g;
    b -= c.b;
    return *this;
}
Color Color::operator*(float t) const {
    return Color(r * t, g * t, b * t);
}
Color Color::operator*=(float t) {
    r *= t;
    g *= t;
    b *= t;
    return *this;
}
Color Color::operator*(const Color& c) const {
    return Color(r * c.r, g * c.g, b * c.b);
}
Color Color::operator*=(const Color& c) {
    r *= c.r;
    g *= c.g;
    b *= c.b;
    return *this;
}
Color Color::operator/(float t) const {
    return Color(r / t, g / t, b / t);
}
Color Color::operator/=(float t) {
    r /= t;
    g /= t;
    b /= t;
    return *this;
}
bool Color::operator==(const Color& c) {
    return r == c.r && g == c.g && b == c.b;
}
bool Color::operator!=(const Color& c) {
    return r != c.r || g != c.g || b != c.b;
}
void Color::clamp() {
    if (r < 0) r = 0; else if (r > 1) r = 1;
    if (g < 0) g = 0; else if (g > 1) g = 1;
    if (b < 0) b = 0; else if (b > 1) b = 1;
}

Color Color::clamped() const {
    return Color(r < 0 ? 0 : (r > 1 ? 1 : r),
                 g < 0 ? 0 : (g > 1 ? 1 : g),
                 b < 0 ? 0 : (b > 1 ? 1 : b));
}

int Color::toInt(float c) const {
    return static_cast<int>(255.0f * c);
}

void Color::invert() {
    r = 1 - r;
    g = 1 - g;
    b = 1 - b;
}

Color Color::inverted() const {
    return Color(1 - r, 1 - g, 1 - b);
}

Color Color::corrected() const {
    
    constexpr float gamma = 2.2f;
    constexpr float invGamma = 1.0f / gamma;

    float rCorrected = pow(Utilities::clamp(r, 0.0f, 1.0f), invGamma);
    float gCorrected = pow(Utilities::clamp(g, 0.0f, 1.0f), invGamma);
    float bCorrected = pow(Utilities::clamp(b, 0.0f, 1.0f), invGamma);

    return Color(rCorrected, gCorrected, bCorrected);
}

Color Color::byteColorFormat() const {
    int rInt = static_cast<int>(255.999f * r);
    int gInt = static_cast<int>(255.999f * g);
    int bInt = static_cast<int>(255.999f * b);

    return Color(rInt, gInt, bInt);
}

float Color::lengthSquared() const {
    return r*r + g*g + b*b;
}

float Color::length() const {
    return sqrt(r*r + g*g + b*b);
}

float Color::luminance() const {
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

float Color::maxComponent() const {
    return max(max(r, g), b);
}

float Color::minComponent() const {
    return min(min(r, g), b);
}