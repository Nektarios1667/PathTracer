#pragma once
#include <random>
#include <Vector3.h>

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
}