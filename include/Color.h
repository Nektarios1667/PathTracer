#pragma once
#include <fstream>
#include <istream>

using namespace std;

struct Color {
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
    friend std::istream& operator>>(std::istream& in, Color& c);
    friend std::ostream& operator<<(std::ostream& out, const Color& c);

    void clamp();
    void invert();

    Color inverted() const;
    Color clamped() const;

    int toInt(float c) const;
    Color corrected() const;
    Color byteColorFormat() const;
    float maxComponent() const;
    float minComponent() const;
    

    float length() const;
    float lengthSquared() const;

    float luminance() const;

    static Color lerp(const Color& a, const Color& b, float weight) {
        return a * (1 - weight) + b * weight;
    }
};

inline Color operator*(float f, const Color& c) {
    return c * f;
}