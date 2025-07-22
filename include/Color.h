#pragma once
#include <fstream>

using namespace std;

class Color {
public:
    float r, g, b;

    Color();
    Color(float rgb);
    Color(float red, float green, float blue);

    Color operator+(const Color& c) const;
    Color operator+=(const Color& c);
    Color operator-(const Color& c) const;
    Color operator-=(const Color& c);
    Color operator*(float t) const;
    Color operator*=(float t);
    Color operator*(const Color& c) const;
    Color operator*=(const Color& c);
    Color operator/(float t) const;
    Color operator/=(float t);
    bool operator==(const Color& c);
    bool operator!=(const Color& c);

    void clamp();
    void invert();

    Color inverted() const;
    Color clamped() const;

    int toInt(float c) const;
    Color corrected() const;
    Color byteColorFormat() const;

    float length() const;
    float lengthSquared() const;

    float luminance() const;

    static Color lerp(Color a, Color b, float weight) {
        return a * (1 - weight) + b * weight;
    }
};

inline Color operator*(float f, const Color& c) {
    return c * f;
}