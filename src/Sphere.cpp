#include "Sphere.h"
#include "Color.h"
#include "Ray.h"
#include "Utilities.h"
#include "lodepng.h"
#include <algorithm>

Sphere::Sphere() : radius(1), radiusSquared(1), center(Vector3()) {
    material = std::make_shared<Material>(Color(), Color(), 0.0f, 1.0f, 1.0f);
}
Sphere::Sphere(const Vector3& center, float radius, shared_ptr<Material> mat) : center(center), radius(radius), radiusSquared(radius*radius) {
    material = mat;
}
bool Sphere::intersectsRay(const Ray& ray, double& outT) const {
    Vector3 oc = center - ray.origin;

    double h = oc.dot(ray.direction);
    double c = oc.lengthSquared() - radiusSquared;
    double discriminant = h*h - c;

    if (discriminant < Utilities::EPSILON) return false;

    double sqrtD = sqrt(discriminant);
    double t1 = h - sqrtD;
    double t2 = h + sqrtD;

    double t = (t1 >= Utilities::EPSILON) ? t1 : ((t2 >= Utilities::EPSILON) ? t2 : -1);
    if (t < Utilities::EPSILON) return false;

    outT = t;
    return true;
}
Vector3d Sphere::getNormalAt(const Vector3d& point, const Vector3d& dir) const {
    Vector3d normal = (point - center).normalized();
    return normal;
}

AABB Sphere::getBoundingBox() const {
    return { center - Vector3(radius), center + Vector3(radius) };
}

//std::tuple<float, float> Sphere::getUV(const Vector3& point) const {
//    Vector3 d = (point - center).normalized();
//    float u = 0.5f + atan2(d.z, d.x) / (2 * Utilities::PI);
//    float v = 0.5f - asin(d.y) / Utilities::PI;
//    return { u, v };
//}
//
//Color Sphere::getUVColor(const Vector3& point) const {
//    auto uv = getUV(point);
//    float u = std::get<0>(uv);
//    float v = std::get<1>(uv);
//    int x = (int)(u * imageWidth);
//	int y = (int)((1 - v) * imageHeight); // Flip v for image coordinates
//    int i = 4 * (y * imageWidth + x);
//	int r, g, b, a;
//	r = texture[i];
//	g = texture[i + 1];
//	b = texture[i + 2];
//	a = texture[i + 3];
//
//	return Color(r / 255.0f, g / 255.0f, b / 255.0f);
//}


