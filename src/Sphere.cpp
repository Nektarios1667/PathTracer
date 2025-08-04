#include "Sphere.h"
#include "Color.h"
#include "Ray.h"
#include "Utilities.h"
#include "lodepng.h"
#include <algorithm>

Sphere::Sphere() : radius(1), radiusSquared(1), center(Vector3()) {
    material = std::make_shared<Material>(Color(), Color(), 0.0f, 1.0f);
}
Sphere::Sphere(const Vector3& center, float radius, shared_ptr<Material> mat) : center(center), radius(radius), radiusSquared(radius*radius) {
    material = mat;
}
bool Sphere::intersectsRay(const Ray& ray, float& outT) const {
    Vector3 oc = center - ray.origin;

    float h = oc.dot(ray.direction);
    float c = oc.lengthSquared() - radiusSquared;
    float discriminant = h*h - c;

    if (discriminant < 0.0f) return false;

    float sqrtD = sqrt(discriminant);
    float t1 = h - sqrtD;
    float t2 = h + sqrtD;

    float t = (t1 >= Utilities::EPSILON) ? t1 : ((t2 >= Utilities::EPSILON) ? t2 : -1);
    if (t < 0.0f) return false;

    outT = t;
    return true;
}
Vector3 Sphere::getNormalAt(const Vector3& point) const {
    return (point - center).normalized();
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


