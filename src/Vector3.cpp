#include "Vector3.h"

Vector3::Vector3() : x(0), y(0), z(0) {}
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
    return Vector3(x, y, z) / length();
}

void Vector3::normalize() {
    float len = Vector3::length();
    x /= len;
    y /= len;
    z /= len;
}

