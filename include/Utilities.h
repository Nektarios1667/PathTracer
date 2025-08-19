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

namespace Utilities {
    constexpr float EPSILON = 0.0001f;
    constexpr float PI = 3.1415926f;
    inline float clamp(float value, float min, float max) {
        return value < min ? min : (value > max ? max : value);
    }

    float randomFloat();
    Color randomColor();
    Vector3 randomInUnitSphere();
    Vector3 randomCosineHemisphere(const Vector3& normal);

    std::vector<std::unique_ptr<Hittable>> readObjFile(const std::string& filename, std::shared_ptr<Material> meshMaterial, float scale = 1, Vector3 offset = {});
    SceneSetup readTrcFile(const std::string& filename, std::vector<string> layers = {});
}
