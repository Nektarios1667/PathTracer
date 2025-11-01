#include <cmath>
#include "Vector3.h"
#include "Camera.h"
#include "Ray.h"
#include <Hittable.h>
#include <cfloat>
#include "Utilities.h"
#include "Constants.h"
#include "PixelData.h"
#include "Triangle.h"
#include <BVHNode.h>
#include <algorithm>

Camera::Camera(const Vector3d& from, const Vector3d& to, const Vector3d& vup, float verticalFov, float aspect)
    : from(from), to(to), vup(vup), verticalFov(verticalFov), aspect(aspect)
{
    updateCamera();
}

void Camera::updateCamera() {
    float theta = verticalFov * Utilities::PI / 180.0f;
    float h = tan(theta / 2.0f);
    float viewportHeight = 2.0f * h;
    float viewportWidth = aspect * viewportHeight;

    Vector3d w = (from - to).normalized();
    Vector3d u = vup.cross(w).normalized();
    Vector3d v = w.cross(u);

    horizontal = u * viewportWidth;
    vertical = v * viewportHeight;
    lowerLeftCorner = from - horizontal * 0.5f - vertical * 0.5f - w;

    viewDirection = (to - from).normalized();
}

Ray Camera::getRay(float u, float v) const {
    Vector3d imagePoint = lowerLeftCorner + horizontal * u + vertical * v;
    Vector3d direction = (imagePoint - from).normalized();
    return Ray(from, direction);
}

Color Camera::getSkybox(const Ray& ray) const {
    float weight = (ray.direction.y + 1) / 2;
    return Color::lerp(Color(1, 1, 1), Color(0, .25f, .57f), weight) * 1.5f;
}

inline Vector3d Camera::refract(const Vector3d& v, const Vector3d& n, double etaRatio, bool& tir) const {
    double cosi = Utilities::clamp(v.dot(n), -1.0, 1.0);
    double k = 1.0 - etaRatio * etaRatio * (1.0 - cosi * cosi);
    if (k < 0.0) {
        tir = true;
        return reflect(v, n);
    }
    tir = false;
    return v * etaRatio + n * (etaRatio * cosi - sqrt(k));
}

inline Vector3d Camera::reflect(const Vector3d& v, const Vector3d& n) const {
    return v - n * 2.0f * v.dot(n);
}


inline float schlickDielectric(float cosine, float idx) {
    float r0 = (1 - idx) / (1 + idx);
    r0 *= r0; // square
    return r0 + (1 - r0) * std::powf(1 - cosine, 5);
}

inline Color schlickGeneric(float cosine, Color F0) {
    return F0 + (Color(1) - F0) * std::powf(1 - cosine, 5);
}

const Hittable* traverseBVH(const BVHNode* node, const Ray& ray, double& closestT, int& checks) {
    double boxT;
    if (!node || !node->bounds.rayHit(ray, boxT) || boxT > closestT) {
        return nullptr;
    }

    if (node->isLeaf()) {
        double t;
        if (node->object->intersectsRay(ray, t) && t < closestT) {
            closestT = t;
            return node->object;
        }
        return nullptr;
    }
    checks++;

    double leftBoxT = DBL_MAX, rightBoxT = DBL_MAX;
    node->left->bounds.rayHit(ray, leftBoxT);
    node->right->bounds.rayHit(ray, rightBoxT);

    BVHNode* firstNode = node->left.get();
    BVHNode* secondNode = node->right.get();
    double firstBoxT = leftBoxT;
    double secondBoxT = rightBoxT;

    if (rightBoxT < leftBoxT) {
        std::swap(firstNode, secondNode);
        std::swap(firstBoxT, secondBoxT);
    }

    double firstT = closestT;
    double secondT = closestT;
    const Hittable* first = traverseBVH(firstNode, ray, firstT, checks);
    const Hittable* second = nullptr;

    if (secondBoxT < firstT)
        second = traverseBVH(secondNode, ray, secondT, checks);

    // Compare closer
    if (first && second) {
        if (firstT < secondT) {
            closestT = firstT;
            return first;
        } else {
            closestT = secondT;
            return second;
        }
    } else if (first) { // Only left
        closestT = firstT;
        return first;
    } else if (second) { // Only right
        closestT = secondT;
        return second;
    }

    return nullptr;
}

Color colorThroughDielectric(Color glassColor, double distance, float scale = .5f) {
    float r = Utilities::clamp(glassColor.r, 0.01f, 1.0f);
	float g = Utilities::clamp(glassColor.g, 0.01f, 1.0f);
    float b = Utilities::clamp(glassColor.b, 0.01f, 1.0f);

    Color result;
    result.r = std::pow(r, scale * distance);
    result.g = std::pow(g, scale * distance);
    result.b = std::pow(b, scale * distance);

	return result;
}

const Hittable* Camera::getHitObject(const Ray& ray, const BVHNode* bvhRoot, double& outT, int& outChecks) const {
    double closestT = DBL_MAX;
    const Hittable* hitObject = traverseBVH(bvhRoot, ray, closestT, outChecks);
    outT = closestT;
    return hitObject;
}

PixelData Camera::traceRay(const Ray& ray, const BVHNode* bvhRoot, int depth) const {
    // Get hit object
    double t;
    int c = 0;
    const Hittable* hitObject = Camera::getHitObject(ray, bvhRoot, t, c);

    // Skybox
    if (!hitObject)
        return { Camera::getSkybox(ray), FLT_MAX, Vector3d(), c };

    // Hit data
    Vector3d hitPoint = ray.at(t);
    Vector3d normal = hitObject->getNormalAt(hitPoint, ray.direction);
    bool exiting = ray.direction.dot(normal) > 0;
    if (exiting) normal = -normal;

    // Emmission
    if (hitObject->material->emission.maxComponent() > Utilities::EPSILON) {
        return { hitObject->material->emission, t, normal, c };
    }

    Color attenuation = Color(1);

    // Diffuse, reflect, or refract based on material and randomness
    Ray bounced;
    // Dialectric
    if (hitObject->material->isDielectric()) {
        // Get reflect or refract
        double etai = exiting ? hitObject->material->refractiveIndex : 1.0;
        double etat = exiting ? 1.0 : hitObject->material->refractiveIndex;
        double etaRatio = etai / etat;

        float cosTheta = std::fminf((-ray.direction).dot(normal), 1.0f); // angle
        float reflectProbability = schlickDielectric(cosTheta, etaRatio);

        // Reflect
        if (Utilities::randomFloat() < reflectProbability) {
            Vector3d reflectedDir = reflect(ray.direction, normal) + Utilities::randomInUnitSphere() * hitObject->material->roughness;
            bounced = Ray(hitPoint + normal * Utilities::EPSILON, reflectedDir.normalized());
        }
        // Refract
        else {
            bool tir;
            Vector3d refractedDir = refract(ray.direction, normal, etaRatio, tir) + Utilities::randomInUnitSphere() * hitObject->material->roughness;
            bounced = Ray(hitPoint - normal * 1e-5, refractedDir.normalized());

            // Exiting non-clear dialectric
            if (exiting) {
                attenuation *= colorThroughDielectric(hitObject->material->albedo, t);
            }
        }
    }
    // Diffuse
    else if (Utilities::randomFloat() > hitObject->material->reflectivity) {
        // Diffuse
        bounced = Ray(hitPoint + normal * Utilities::EPSILON, Utilities::randomCosineHemisphere(normal));
        attenuation *= hitObject->material->albedo;
    }
    // Reflect
    else {
        // Reflect
        Vector3d reflectedDir = reflect(ray.direction, normal) + Utilities::randomInUnitSphere() * hitObject->material->roughness;
        bounced = Ray(hitPoint + normal * Utilities::EPSILON, reflectedDir.normalized());

        // Schlick
        double cosTheta = std::max(0.0, normal.dot(-ray.direction));
        attenuation *= schlickGeneric(cosTheta, hitObject->material->albedo * .9f);
    }

    // Russian roulette
    if (depth > MIN_DEPTH) {
        // Dielectric
        if (hitObject->material->isDielectric()) {
            constexpr float p = 0.95f; // set probability
            if (depth > MAX_DEPTH || Utilities::randomFloat() > p)  return { Color(), DBL_MAX, Vector3d(), c };
			attenuation /= p;
        }
        // Diffusive/reflective
        else {
            float p = max(max(attenuation.r, attenuation.g), attenuation.b);
            p = Utilities::clamp(p, .1f, 1.0f);
            if (depth > MAX_DEPTH || Utilities::randomFloat() > p) return { Color(), DBL_MAX, Vector3d(), c };
            attenuation /= p;
        }
    }

    // Trace bounce
    PixelData recursive = Camera::traceRay(bounced, bvhRoot, depth + 1);
    Color final = recursive.color * attenuation;

    return { final, t, normal, c};
}

PixelData Camera::tracePixel(int x, int y, int width, int height, const BVHNode* bvhRoot) const {
    // Setup color
    Color colorSum = Color();
    Color colorSumSq = Color();
    double depthSum = 0;
    Vector3d normalSum = Vector3d();
    int checksSum = 0;

    // Adaptively sample
    int samples = 0;
    while (samples < MAX_SAMPLES) {
        // Create ray
        float offsetX = MAX_SAMPLES <= 1 ? 0 : Utilities::randomFloat() - .5f;
        float offsetY = MAX_SAMPLES <= 1 ? 0 : Utilities::randomFloat() - .5f;
        float u = float(x + offsetX) / float(width - 1);
        float v = 1 - float(y + offsetY) / height;
        const Ray ray = Camera::getRay(u, v);

        // Add sample to sums
        PixelData sample = traceRay(ray, bvhRoot);
        samples++;

        // Sums
        colorSum += sample.color;
        colorSumSq += sample.color * sample.color;
        depthSum += sample.depth;
        normalSum += sample.normal;
        checksSum += sample.checks;

        // Check variance for early exit
        if (samples >= MIN_SAMPLES) {
            Color average = colorSum / samples;
            Color variance = (colorSumSq / samples) - (average * average);
            if (variance.luminance() < SAMPLE_THRESHOLD) { break; }
        }
    }

    // Return final color
    Color finalColor = colorSum / samples;
    float finalDepth = depthSum / samples;
    Vector3d finalNormal = (normalSum / samples).normalized();
    int finalChecks = checksSum / samples;
    return { finalColor, finalDepth, finalNormal, finalChecks, samples };
}

vector<unsigned char> Camera::getRenderOutput(const vector<PixelData>& pixels, RenderType renderType) const {
    vector<unsigned char> colorData(IMAGE_WIDTH * IMAGE_HEIGHT * 4);

    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            int i = y * IMAGE_WIDTH + x;
            int p = i * 4;

            if (renderType == RenderType::Light) {
                Color color = pixels[i].color.corrected().byteColorFormat();
                colorData[p] = color.r;
                colorData[p + 1] = color.g;
                colorData[p + 2] = color.b;
            } else if (renderType == RenderType::Depth) {
                float depth = Utilities::clamp((pixels[i].depth - NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE), 0, 1);
                colorData[p] = static_cast<int>(depth * 255);
                colorData[p + 1] = static_cast<int>(depth * 255);
                colorData[p + 2] = static_cast<int>(depth * 255);
            } else if (renderType == RenderType::Normals) {
                Vector3 normal = pixels[i].normal;
                colorData[p] = static_cast<int>((normal.x * 0.5f + 0.5f) * 255.0f);
                colorData[p + 1] = static_cast<int>((normal.y * 0.5f + 0.5f) * 255.0f);
                colorData[p + 2] = static_cast<int>((normal.z * 0.5f + 0.5f) * 255.0f);
            } else if (renderType == RenderType::BVH) {
                float checks = min(max(pixels[i].checks / 30.0f, 0.0f), 1.0f);
                colorData[p] = static_cast<int>(checks * 255);
                colorData[p + 1] = static_cast<int>(checks * 255);
                colorData[p + 2] = static_cast<int>(checks * 255);
            } else if (renderType == RenderType::Samples) {
                float samples = (pixels[i].samples - MIN_SAMPLES) / (float)(MAX_SAMPLES);
                colorData[p] = static_cast<int>(samples * 255);
                colorData[p + 1] = static_cast<int>(samples * 255);
                colorData[p + 2] = static_cast<int>(samples * 255);
            }

            colorData[p + 3] = 255;
        }
    }

    return colorData;
}
