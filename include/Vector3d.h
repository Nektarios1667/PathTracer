#pragma once
#include <cmath>
#include <istream>
#include <ostream>

class Vector3;

class Vector3d {
public:
    double x;
    double y;
    double z;

    Vector3d();
    Vector3d(double v);
    Vector3d(double x, double y, double z);
	Vector3d(float x, float y, float z);
	Vector3d(const Vector3& v);

    double lengthSquared() const;
    double length() const;

    double distanceSquared(const Vector3d& v) const;
    double distance(const Vector3d& v) const;

    Vector3d normalized() const;
    Vector3d inversed() const;
	Vector3 toVector3() const;

    Vector3d operator+(const Vector3d& v) const;
    Vector3d operator+=(const Vector3d& v);
    Vector3d operator-(const Vector3d& v) const;
    Vector3d operator-() const;
    Vector3d operator*(double t) const;
    Vector3d operator/(double t) const;
    double operator[](int idx) const;
    friend std::istream& operator>>(std::istream& in, Vector3d& v);
    friend std::ostream& operator<<(std::ostream& out, const Vector3d& v);

    void normalize();
    void inverse();

    double dot(const Vector3d& v) const;
    Vector3d cross(const Vector3d& v) const;
};
