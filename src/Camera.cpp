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
}

const Hittable* traverseBVH(const unique_ptr<BVHNode>& node, const Ray& ray, float& closestT, int& checks) {
    float boxT;
    if (!node || !node->bounds.rayHit(ray, boxT) || boxT > closestT) {
        return nullptr;
    }

    if (node->isLeaf()) {
        float t;
        if (node->object->intersectsRay(ray, t) && t < closestT) {
            closestT = t;
            return node->object;
        }
        return nullptr;
    }
    checks++;

    float leftT = closestT;
    float rightT = closestT;
    const Hittable* hitLeft = traverseBVH(node->left, ray, leftT, checks);
    const Hittable* hitRight = traverseBVH(node->right, ray, rightT, checks);

    // Compare closer
    if (hitLeft && hitRight) {
        if (leftT < rightT) {
            closestT = leftT;
            return hitLeft;
        } else {
            closestT = rightT;
            return hitRight;
        }
    } else if (hitLeft) { // Only left
        closestT = leftT;
        return hitLeft;
    } else if (hitRight) { // Only right
        closestT = rightT;
        return hitRight;
    }

    return nullptr;
}

const Hittable* Camera::getHitObject(const Ray& ray, const unique_ptr<BVHNode>& bvhRoot, float& outT, int& outChecks) const {
    float closestT = DBL_MAX;
    const Hittable* hitObject = traverseBVH(bvhRoot, ray, closestT, outChecks);
    outT = closestT;
    return hitObject;
}

PixelData Camera::traceRay(const Ray& ray, const unique_ptr<BVHNode>& bvhRoot, int depth) const {
    // Get hit object
    float t;
    int c = 0;
    const Hittable* hitObject = Camera::getHitObject(ray, bvhRoot, t, c);

    // Skybox
    if (!hitObject) return { Camera::getSkybox(ray), FLT_MAX, Vector3(), c };

    // Hit data
    Vector3 hitPoint = ray.at(t);
    Vector3 normal = hitObject->getNormalAt(hitPoint);

    // Emmission
    if (hitObject->material.emission.maxComponent() > Utilities::EPSILON) {
        return { hitObject->material.emission, t, normal, c };
    }

    // Diffuse or reflect based on material and randomness
    Ray bounced;
    if (Utilities::randomFloat() > hitObject->material.reflectivity) {
        // Diffuse
        bounced = Ray(hitPoint + normal * Utilities::EPSILON * fabsf(t), Utilities::randomCosineHemisphere(normal));
    } else {
        // Reflect
        Vector3 reflectedDir = ray.direction - normal * 2 * ray.direction.dot(normal) + Utilities::randomInUnitSphere() * hitObject->material.roughness;
        bounced = Ray(hitPoint + reflectedDir * Utilities::EPSILON * fabsf(t), reflectedDir.normalized());
    }

    Color attenuation = hitObject->material.albedo;

    // Russian roulette
    if (depth > MIN_DEPTH) {
        float p = max(max(attenuation.r, attenuation.g), attenuation.b);
        p = Utilities::clamp(p, .1f, 1.0f);
        if (depth > MAX_DEPTH || Utilities::randomFloat() > p) return { Color(), FLT_MAX, Vector3(), c };
        attenuation /= p;
    }

    // Trace bounce
    PixelData recursive = Camera::traceRay(bounced, bvhRoot, depth + 1);
    Color final = hitObject->material.emission + recursive.color * attenuation;

    return { final, t, normal, c };
}

PixelData Camera::tracePixel(int x, int y, int width, int height, const std::unique_ptr<BVHNode>& bvhRoot) const {
    // Setup color
    Color colorSum = Color();
    Color colorSumSq = Color();
    float depthSum = 0;
    Vector3 normalSum = Vector3();
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
    Vector3 finalNormal = (normalSum / samples).normalized();
    int finalChecks = checksSum / samples;
    return { finalColor, finalDepth, finalNormal, finalChecks };
}

void Camera::bilateralBlurHorizontal(vector<PixelData>& pixels, vector<PixelData>& temp) const {
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            float totalWeight = 0;
            Color finalColor = Color();
            float centerDepth = pixels[y * IMAGE_WIDTH + x].depth;

            for (int dx = -BILATERAL_RADIUS; dx <= BILATERAL_RADIUS; dx++) {
                int tx = x + dx;
                if (tx < 0 || tx >= IMAGE_WIDTH) continue;
                
                PixelData neighbor = pixels[y * IMAGE_WIDTH + tx];

                float deltaDepth = abs(centerDepth - neighbor.depth);
                float depthWeight = 1 / (deltaDepth + 1);
                float distWeight = 1 / (float)(abs(dx) + 1);
                float weight = depthWeight * distWeight;

                finalColor += neighbor.color * weight;
                totalWeight += weight;
            }

            PixelData& result = temp[y * IMAGE_WIDTH + x];
            result = pixels[y * IMAGE_WIDTH + x]; // Keep other data
            result.color = totalWeight != 0 ? finalColor / totalWeight : result.color;
        }
    }
}

void Camera::bilateralBlurVertical(vector<PixelData>& pixels, vector<PixelData>& temp) const {
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            float totalWeight = 0;
            Color finalColor = Color();
            float centerDepth = pixels[y * IMAGE_WIDTH + x].depth;

            for (int dy = -BILATERAL_RADIUS; dy <= BILATERAL_RADIUS; dy++) {
                int ty = y + dy;
                if (ty < 0 || ty >= IMAGE_HEIGHT) continue;
                
                PixelData neighbor = pixels[ty * IMAGE_WIDTH + x];

                float deltaWeight = abs(centerDepth - neighbor.depth);
                float depthWeight = 1 / (deltaWeight + 1);
                float distWeight = 1 / (float)(abs(dy) + 1);
                float weight = depthWeight * distWeight;

                finalColor += neighbor.color * weight;
                totalWeight += weight;
            }

            PixelData& result = temp[y * IMAGE_WIDTH + x];
            result = pixels[y * IMAGE_WIDTH + x]; // Keep other data
            result.color = totalWeight != 0 ? finalColor / totalWeight : result.color;
        }
    }
}

vector<unsigned char> Camera::getRenderOutput(vector<PixelData>& pixels) const {
    vector<unsigned char> colorData(IMAGE_WIDTH * IMAGE_HEIGHT * 4);

    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            int i = y * IMAGE_WIDTH + x;
            int p = i * 4;

            if (RENDER_TYPE == RenderType::Light) {
                Color color = pixels[i].color.corrected().byteColorFormat();
                colorData[p] = color.r;
                colorData[p + 1] = color.g;
                colorData[p + 2] = color.b;
            } else if (RENDER_TYPE == RenderType::Depth) {
                float depth = Utilities::clamp((pixels[i].depth - NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE), 0, 1);
                colorData[p] = static_cast<int>(depth * 255);
                colorData[p + 1] = static_cast<int>(depth * 255);
                colorData[p + 2] = static_cast<int>(depth * 255);
            } else if (RENDER_TYPE == RenderType::Normal) {
                Vector3 normal = pixels[i].normal;
                colorData[p] = static_cast<int>((normal.x * 0.5f + 0.5f) * 255.0f);
                colorData[p + 1] = static_cast<int>((normal.y * 0.5f + 0.5f) * 255.0f);
                colorData[p + 2] = static_cast<int>((normal.z * 0.5f + 0.5f) * 255.0f);
            } else if (RENDER_TYPE == RenderType::BVH) {
                float checks = min(max(pixels[i].checks / 100.0f, 0.0f), 1.0f);
                colorData[p] = static_cast<int>(checks * 255);
                colorData[p + 1] = static_cast<int>(checks * 255);
                colorData[p + 2] = static_cast<int>(checks * 255);
            }

            colorData[p + 3] = 255;
        }
    }

    return colorData;
}
