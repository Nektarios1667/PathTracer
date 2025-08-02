#pragma once
#include <cmath>
#include <istream>
#include <ostream>

class Vector3 {
public:
    float x;
    float y;
    float z;

    Vector3();
    Vector3(float v);
    Vector3(float x, float y, float z);

    float lengthSquared() const;
    float length() const;

    float distanceSquared(const Vector3& v) const;
    float distance(const Vector3& v) const;

    Vector3 normalized() const;
    Vector3 inversed() const;

    Vector3 operator+(const Vector3& v) const;
    Vector3 operator+=(const Vector3& v);
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator-() const;
    Vector3 operator*(float t) const;
    Vector3 operator/(float t) const;
    float operator[](int idx) const;
    friend std::istream& operator>>(std::istream& in, Vector3& v);
    friend std::ostream& operator<<(std::ostream& out, const Vector3& v);

    void normalize();
    void inverse();

    float dot(const Vector3& v) const;
    Vector3 cross(const Vector3& v) const;
};
