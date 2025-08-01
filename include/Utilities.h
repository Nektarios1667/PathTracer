#pragma once
#include <random>
#include <sstream>
#include <Vector3.h>
#include <memory>
#include "Triangle.h"
#include <assert.h>
#include <iostream>

namespace Utilities {
    constexpr float EPSILON = 0.0001f;
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
            std::string prefix;
            iss >> prefix;

            // Vertex
            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            } else if (prefix == "f") {
                int i1, i2, i3;
                iss >> i1 >> i2 >> i3;

                triangles.emplace_back(std::make_unique<Triangle>(vertices[i1 - 1], vertices[i2 - 1], vertices[i3 - 1], Material{ Color(1.0f), Color(), 1.0f, 0.0f }
                ));            
            }
        }
        return triangles;
    }
}