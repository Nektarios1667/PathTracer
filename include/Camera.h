#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "Hittable.h"
#include <memory>
#include <vector>
#include "Constants.h"
#include "PixelData.h"
#include "BVHNode.h"
#include "Vector3d.h"

struct HitRecord {
    Hittable* object;
    float t;
    Vector3d point;
    Vector3d normal;
};

class Camera {
    public:
        Vector3d from;
        Vector3d to;
		Vector3d vup;
		float verticalFov;
		float aspect;

        Vector3d horizontal, vertical, lowerLeftCorner;

        Camera(const Vector3d& from, const Vector3d& to, const Vector3d& vup, float verticalFov, float aspect);

		void updateCamera();
        void moveCamera(const Vector3d& move) {
            from += move;
            to += move;

			updateCamera();
        }
        void setCamera(const Vector3d& newFrom) {
			Vector3d delta = newFrom - from;
			from = newFrom;
			to += delta;

            updateCamera();
		}

        Color getSkybox(const Ray& ray) const;
        Ray getRay(float u, float v) const;
        const Hittable* getHitObject(const Ray& ray, const BVHNode* bvhRoot, double& outT, int& outChecks) const;
        Vector3d refract(const Vector3d& v, const Vector3d& n, double eta, bool& tir) const;
        Vector3d reflect(const Vector3d& v, const Vector3d& n) const;
        float schlick(float cosine, float idx) const;

        PixelData traceRay(const Ray& ray, const BVHNode* bvhRoot, int depth = 0) const;
        PixelData tracePixel(int x, int y, int width, int height, const BVHNode* bvhRoot) const;

        vector<unsigned char> getRenderOutput(const vector<PixelData>& pixels, RenderType renderType) const;
};