#include "Utilities.h"
#include "Sphere.h"
#include <random>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Utilities {
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    float randomFloat() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }

    Color randomColor() {
        return Color(randomFloat(), randomFloat(), randomFloat());
    }

    Vector3d randomInUnitSphere() {
        while (true) {
            Vector3 p(randomFloat() * 2 - 1, randomFloat() * 2 - 1, randomFloat() * 2 - 1);
            if (p.lengthSquared() < 1) return p;
        }
    }

    Vector3d randomCosineHemisphere(const Vector3d& normal) {
        double r1 = 2.0 * Utilities::PI * ((double)rand() / RAND_MAX);
        double r2 = (double)rand() / RAND_MAX;
        double r = sqrt(r2);
        double z = sqrt(1.0 - r2);

        Vector3d u = ((fabs(normal.x) > 0.1) ? Vector3d(0.0, 1.0, 0.0) : Vector3d(1.0, 0.0, 0.0)).cross(normal).normalized();
        Vector3d v = normal.cross(u);

        return (u * (cos(r1) * r) + v * (sin(r1) * r) + normal * z).normalized();
    }

    std::vector<std::unique_ptr<Hittable>> readObjFile(const std::string& filename, std::shared_ptr<Material> meshMaterial, float scale, Vector3 offset) {
        std::vector<Vector3> vertices;
        std::vector<std::unique_ptr<Hittable>> triangles;

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x * scale + offset.x, y * scale + offset.y, z * scale + offset.z);
            } else if (prefix == "f") {
                int i1, i2, i3;
                iss >> i3 >> i2 >> i1;
                triangles.emplace_back(std::make_unique<Triangle>(vertices[i1 - 1], vertices[i2 - 1], vertices[i3 - 1], meshMaterial));
            }
        }
        return triangles;
    }
}
