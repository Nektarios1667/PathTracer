#include "Vector3.h"
#include <stdexcept>
#include <iostream>

Vector3::Vector3() : x(0), y(0), z(0) {}
Vector3::Vector3(float v) : x(v), y(v), z(v) {}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vector3::lengthSquared() const {
    return x * x + y * y + z * z;
}

float Vector3::length() const {
    return std::sqrt(lengthSquared());
}

float Vector3::distanceSquared(const Vector3& v) const {
    return (v - Vector3(x, y, z)).lengthSquared();
}
float Vector3::distance(const Vector3& v) const {
    return (v - Vector3(x, y, z)).length();
}

Vector3 Vector3::operator+(const Vector3& v) const {
    return Vector3(x + v.x, y + v.y, z + v.z);
}
Vector3 Vector3::operator+=(const Vector3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
}

Vector3 Vector3::operator-(const Vector3& v) const {
    return Vector3(x - v.x, y - v.y, z - v.z);
}
Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}
Vector3 Vector3::operator*(float t) const {
    return Vector3(x * t, y * t, z * t);
}

Vector3 Vector3::operator/(float t) const {
    return Vector3(x / t, y / t, z / t);
}

float Vector3::operator[](int idx) const {
    if (idx == 0) return x;
    if (idx == 1) return y;
    if (idx == 2) return z;
    return 0;
}

std::istream& operator>>(std::istream& in, Vector3& v)  {
    char comma1, comma2;
    if (!(in >> v.x >> comma1 >> v.y >> comma2 >> v.z)) {
        in.setstate(std::ios::failbit);
        return in;
    }

    if (comma1 != ',' || comma2 != ',') {
        in.setstate(std::ios::failbit);
        return in;
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const Vector3& v) {
    return out << v.x << "," << v.y << "," << v.z;
}

float Vector3::dot(const Vector3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::cross(const Vector3& v) const {
    return Vector3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x
    );
}

Vector3 Vector3::normalized() const {
    if (lengthSquared() == 0) return Vector3();
    return Vector3(x, y, z) / length();
}

void Vector3::normalize() {
    float len = Vector3::length();
    x /= len;
    y /= len;
    z /= len;
}

Vector3 Vector3::inversed() const {
    return Vector3(1 / x, 1 / y, 1 / z);
}

void Vector3::inverse() {
    x = 1 / x;
    y = 1 / y;
    z = 1 / z;
}
