#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Hittable.h"
#include <memory>
#include <vector>
#include "Light.h"

struct HitRecord {
    Hittable* object;
    float t;
    Vector3 point;
    Vector3 normal;
};

class Camera {
    public:
        Vector3 origin;
        Vector3 lowerLeftCorner;
        Vector3 horizontal;
        Vector3 vertical;
        Camera();
        Camera(const Vector3& from, const Vector3& at, const Vector3& vup, float verticalFov, float aspect);

        Color getSkybox(const Ray& ray) const;

        Ray getRay(float u, float v) const;
        const Hittable* getHitObject(const Ray& ray, const vector<unique_ptr<Hittable>>& scene, float& outT) const;
        Color traceRay(const Ray& ray, const vector<unique_ptr<Hittable>>& scene, int depth = 0) const;
        Color tracePixel(int x, int y, int width, int height, const vector<unique_ptr<Hittable>>& scene) const;
};