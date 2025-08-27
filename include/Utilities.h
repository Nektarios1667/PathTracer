#pragma once
#include <Vector3.h>
#include <memory>
#include <vector>
#include "Triangle.h"
#include <unordered_map>

struct SceneSetup {
    std::vector<std::unique_ptr<Hittable>> hittables;
    std::unordered_map<string, std::shared_ptr<Material>> materials;
    Vector3 cameraFrom;
    Vector3 cameraTo;
};

// TODO Get good epsilon value around e-8 is good for glass monkey, but causes ghost images in glass sphere
// Maybe uses multiplier based on where epsilon is being used
// Or have multiple scales like large, small
// Or figure out better logic for self intersections
namespace Utilities {
    constexpr double EPSILON = 1e-8;
    constexpr float PI = 3.1415926f;
    inline float clamp(float value, float min, float max) {
        return value < min ? min : (value > max ? max : value);
    }
    inline double clamp(double value, double min, double max) {
        return value < min ? min : (value > max ? max : value);
	}

    float randomFloat();
    Color randomColor();
    Vector3d randomInUnitSphere();
    Vector3d randomCosineHemisphere(const Vector3d& normal);

    std::vector<std::unique_ptr<Hittable>> readObjFile(const std::string& filename, std::shared_ptr<Material> meshMaterial, float scale = 1, Vector3 offset = {});
    SceneSetup readTrcFile(const std::string& filename, std::vector<string> layers = {});
}
