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
};
constexpr int RESOLUTION = (int)Resolution::Low;
constexpr float ASPECT = 16.0f / 9.0f; // width / height
constexpr int IMAGE_HEIGHT = (int)(RESOLUTION / ASPECT);
constexpr unsigned MIN_DEPTH = 2; // minimum bounces
constexpr unsigned MAX_DEPTH = 4; // max bounced to prevent too much recursion
constexpr unsigned MIN_SAMPLES = 1; // minimum adaptive samples
constexpr unsigned MAX_SAMPLES = 1; // cutoff for adaptive sampling
constexpr float SAMPLE_THRESHOLD = 0.0001f; // threshold for dynamic sampling
constexpr unsigned FOV = 70; // field of view
constexpr int BILATERAL_RADIUS = 0; // radius of bilateral filter blur

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 10.0f;

// Debug
constexpr RenderType RENDER_TYPE = RenderType::Light;