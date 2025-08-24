#include "Vector3d.h"
#include <stdexcept>
#include <iostream>
#include <Vector3.h>

Vector3d::Vector3d() : x(0), y(0), z(0) {}
Vector3d::Vector3d(double v) : x(v), y(v), z(v) {}
Vector3d::Vector3d(double x, double y, double z) : x(x), y(y), z(z) {}
Vector3d::Vector3d(float x, float y, float z) : x((double)x), y((double)y), z((double)z) {}
Vector3d::Vector3d(const Vector3& v) : x((double)v.x), y((double)v.y), z((double)v.z) {}

double Vector3d::lengthSquared() const {
    return x * x + y * y + z * z;
}

double Vector3d::length() const {
    return std::sqrt(lengthSquared());
}

double Vector3d::distanceSquared(const Vector3d& v) const {
    return (v - Vector3d(x, y, z)).lengthSquared();
}
double Vector3d::distance(const Vector3d& v) const {
    return (v - Vector3d(x, y, z)).length();
}

Vector3 Vector3d::toVector3() const {
    return Vector3(x, y, z);
}

Vector3d Vector3d::operator+(const Vector3d& v) const {
    return Vector3d(x + v.x, y + v.y, z + v.z);
}
Vector3d Vector3d::operator+=(const Vector3d& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector3d Vector3d::operator-(const Vector3d& v) const {
    return Vector3d(x - v.x, y - v.y, z - v.z);
}
Vector3d Vector3d::operator-() const {
    return Vector3d(-x, -y, -z);
}
Vector3d Vector3d::operator*(double t) const {
    return Vector3d(x * t, y * t, z * t);
}

Vector3d Vector3d::operator/(double t) const {
    return Vector3d(x / t, y / t, z / t);
}

double Vector3d::operator[](int idx) const {
    if (idx == 0) return x;
    if (idx == 1) return y;
    if (idx == 2) return z;
    return 0;
}

std::istream& operator>>(std::istream& in, Vector3d& v)  {
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

std::ostream& operator<<(std::ostream& out, const Vector3d& v) {
    return out << v.x << "," << v.y << "," << v.z;
}

double Vector3d::dot(const Vector3d& v) const {
	double vx = v.x, vy = v.y, vz = v.z;
    return x * vx + y * vy + z * vz;
}

Vector3d Vector3d::cross(const Vector3d& v) const {
    return Vector3d(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x
    );
}

Vector3d Vector3d::normalized() const {
    if (lengthSquared() == 0) return Vector3d();
    return Vector3d(x, y, z) / length();
}

void Vector3d::normalize() {
    double len = Vector3d::length();
    x /= len;
    y /= len;
    z /= len;
}

Vector3d Vector3d::inversed() const {
    return Vector3d(1 / x, 1 / y, 1 / z);
}

void Vector3d::inverse() {
    x = 1 / x;
    y = 1 / y;
    z = 1 / z;
}
