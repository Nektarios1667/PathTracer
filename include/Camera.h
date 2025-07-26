#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Hittable.h"
#include <memory>
#include <vector>
#include "Light.h"
#include "PixelData.h"
#include "BVHNode.h"

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
        const Hittable* getHitObject(const Ray& ray, const BVHNode& bvhRoot, float& outT, int& outChecks) const;
        PixelData traceRay(const Ray& ray, const BVHNode& bvhRoot, int depth = 0) const;
        PixelData tracePixel(int x, int y, int width, int height, const BVHNode& bvhRoot) const;
        void bilateralBlurHorizontal(vector<PixelData>& pixels, vector<PixelData>& temp) const;
        void bilateralBlurVertical(vector<PixelData>& pixels, vector<PixelData>& temp) const;
        vector<unsigned char> getRenderOutput(vector<PixelData>& pixels) const;
};