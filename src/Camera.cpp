#include <cmath>
#include "Vector3.h"
#include "Camera.h"
#include "Ray.h"
#include <Hittable.h>
#include <cfloat>
#include "Utilities.h"
#include "Constants.h"

Camera::Camera() {}

Camera::Camera(const Vector3& from, const Vector3& at, const Vector3& vup, float verticalFov, float aspect) {
    origin = from;

    float theta = verticalFov * M_PI / 180.0f;
    float h = tan(theta / 2.0f);
    float viewportHeight = 2.0f * h;
    float viewportWidth = aspect * viewportHeight;

    Vector3 w = (from - at).normalized();
    Vector3 u = vup.cross(w).normalized();
    Vector3 v = w.cross(u);

    horizontal = u * viewportWidth;
    vertical = v * viewportHeight;
    lowerLeftCorner = origin - horizontal * 0.5f - vertical * 0.5f - w;
}

Ray Camera::getRay(float u, float v) const {
    Vector3 imagePoint = lowerLeftCorner + horizontal * u + vertical * v;
    Vector3 direction = (imagePoint - origin).normalized();
    return Ray(origin, direction);
}

Color Camera::getSkybox(const Ray& ray) const {
    float weight = (ray.direction.y + 1) / 2;
    return Color::lerp(Color(1, 1, 1), Color(0, .5f, 1.0f), weight);
    // return Color(0.53f, 0.81f, 0.92f);
    return Color((ray.direction.x + 1) / 2, (ray.direction.y + 1) / 2, (ray.direction.z + 1) / 2).corrected().inverted();  // sky box
}

const Hittable* Camera::getHitObject(const Ray& ray, const vector<unique_ptr<Hittable>>& scene, float& outT) const {
    // Get closest object
    float closestT = FLT_MAX;
    const Hittable* hitObject = nullptr;
    for (const auto& item : scene) {
        float t;
        if (item->intersectsRay(ray, t) && t < closestT) {
            hitObject = item.get();
            closestT = t;
        }
    }
    outT = closestT;
    return hitObject;
}

Color Camera::traceRay(const Ray& ray, const vector<unique_ptr<Hittable>>& scene, int depth) const {
    // Get hit object
    float t;
    const Hittable* hitObject = Camera::getHitObject(ray, scene, t);

    // Skybox
    if (!hitObject) return Camera::getSkybox(ray);

    // Hit data
    Vector3 hitPoint = ray.at(t);
    Vector3 normal = hitObject->getNormalAt(hitPoint);

    // Diffuse or reflect based on material and randomness
    Ray bounced;
    if (Utilities::randomFloat() > hitObject->material.reflectivity) {
        // Diffuse
        bounced = Ray(hitPoint + normal * Utilities::EPSILON, Utilities::randomCosineHemisphere(normal));
    } else {
        // Reflect
        Vector3 reflectedDir = ray.direction - normal * 2 * ray.direction.dot(normal) + Utilities::randomInUnitSphere() * hitObject->material.roughness;
        bounced = Ray(hitPoint + reflectedDir * Utilities::EPSILON, reflectedDir.normalized());
    }

    Color attenuation = hitObject->material.albedo;

    // Russian roulette
    if (depth > MIN_DEPTH) {
        float p = max(max(attenuation.r, attenuation.g), attenuation.b);
        p = Utilities::clamp(p, .1f, 1.0f);
        if (depth > MAX_DEPTH || Utilities::randomFloat() > p) return Color();
        attenuation /= p;
    }

    // Trace bounce
    Color diffused = Camera::traceRay(bounced, scene, depth + 1);

    return hitObject->material.emission + diffused * attenuation;
}

Color Camera::tracePixel(int x, int y, int width, int height, const vector<unique_ptr<Hittable>>& scene) const {
    // Setup color
    Color colorSum = Color();
    Color colorSumSq = Color();

    // Adaptively sample
    int samples = 0;
    while (samples < MAX_SAMPLES) {
        // Create ray
        float offsetX = Utilities::randomFloat() - .5f;
        float offsetY = Utilities::randomFloat() - .5f;
        float u = float(x + offsetX) / float(width - 1);
        float v = 1 - float(y + offsetY) / height;
        const Ray ray = Camera::getRay(u, v);

        // Add sample to sums
        Color sample = traceRay(ray, scene);
        colorSum += sample;
        colorSumSq += sample * sample;
        samples++;
    
        // Check variance for early exit
        if (samples >= MIN_SAMPLES) {
            Color average = colorSum / samples;
            Color variance = (colorSumSq / samples) - (average * average);
            if (variance.r <= SAMPLE_THRESHOLD &&
                variance.g <= SAMPLE_THRESHOLD &&
                variance.b <= SAMPLE_THRESHOLD) {
                    break;
            }
        }
    }

    // Return final color
    return colorSum / samples;
}
