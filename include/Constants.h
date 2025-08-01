#pragma once

enum class Resolution {
    Low = 640,
    Medium = 1280,
    High = 1920,
    VeryHigh = 3840,
    Ultra = 7680,
};

enum class RenderType {
    Light,
    Normal,
    Depth,
    BVH,
};

constexpr int IMAGE_WIDTH = (int)Resolution::VeryHigh;
constexpr float ASPECT = 16.0f / 9.0f; // width / height
constexpr int IMAGE_HEIGHT = (int)(IMAGE_WIDTH / ASPECT);
constexpr RenderType RENDER_TYPE = RenderType::Light; // "Normal" does not mean "regular" - it means the normal map, aka the direction of the surfaces
constexpr unsigned MIN_DEPTH   = RENDER_TYPE == RenderType::Light ? 5   : 1; // minimum bounces
constexpr unsigned MAX_DEPTH   = RENDER_TYPE == RenderType::Light ? 10  : 1; // max bounced to prevent too much recursion
constexpr unsigned MIN_SAMPLES = RENDER_TYPE == RenderType::Light ? 64  : 1; // minimum adaptive samples
constexpr unsigned MAX_SAMPLES = RENDER_TYPE == RenderType::Light ? 256 : 1; // cutoff for adaptive sampling
constexpr float SAMPLE_THRESHOLD = 0.005f; // threshold for dynamic sampling
constexpr unsigned FOV = 70; // field of view
constexpr int BILATERAL_RADIUS = 0; // radius of bilateral filter blur

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 10.0f;
