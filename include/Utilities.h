#pragma once
#include <random>
#include <sstream>
#include <string>
#include <Vector3.h>
#include <memory>
#include "Triangle.h"

namespace Utilities {
    constexpr float EPSILON = 0.001f;
    constexpr float clamp(float value, float min, float max) {
        return (value < min) ? min : (value > max) ? max : value;
    }
    inline float randomFloat() {
        static std::mt19937 rng(std::random_device{}()); // random engine
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f); // range [0,1)
        return dist(rng); 
    }
    inline Color randomColor() {
        return Color(randomFloat(), randomFloat(), randomFloat());
    }
    inline Vector3 randomInUnitSphere() {
        while (true) {
            Vector3 p = Vector3(randomFloat() * 2 - 1, randomFloat() * 2 - 1, randomFloat() * 2 - 1);
            if (p.lengthSquared() < 1) return p;
        }
    }
    inline Vector3 randomCosineHemisphere(const Vector3& normal) {
        double r1 = 2.0 * M_PI * ((double)rand() / RAND_MAX);
        double r2 = (double)rand() / RAND_MAX;
        double r = sqrt(r2);
        double z = sqrt(1.0 - r2);

        Vector3 u = ((fabs(normal.x) > 0.1) ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).cross(normal).normalized();
        Vector3 v = normal.cross(u);

        return (u * (cos(r1) * r) + v * (sin(r1) * r) + normal * z).normalized();
    }
    inline vector<unique_ptr<Hittable>> readObjFile(string filename) {
        vector<Vector3> vertices;
        vector<unique_ptr<Hittable>> triangles;

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            string prefix;
            iss >> prefix;

            // Vertices
            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            }
            // Faces
            else if (prefix == "f") {
                int i0, i1, i2;
                iss >> i0 >> i1 >> i2;
                triangles.emplace_back(make_unique<Triangle>(vertices[i0 - 1], vertices[i1 - 1], vertices[i2 - 1], Material{ Color(1.0f), Color(), 0.0f, 1.0f }));
            }
        }
        return triangles;
    }
}