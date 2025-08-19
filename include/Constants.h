#pragma once
#include <map>

enum class Resolution {
    VeryLow = 320,
    Low = 640,
    Medium = 1280,
    High = 1920,
    VeryHigh = 3840,
    Ultra = 7680,
};

enum class RenderType {
    Light,
    Normals,
    Depth,
    BVH,
    Samples,
    All,
};
const std::map<RenderType, std::string> RenderTypeMap = {
    {RenderType::Light, "light"},
    {RenderType::Normals, "normals"},
    {RenderType::Depth, "depth"},
    {RenderType::BVH, "bvh"},
    {RenderType::Samples, "samples"},
    {RenderType::All, "all"},
};

constexpr int IMAGE_WIDTH = (int)Resolution::Medium;
constexpr float ASPECT = 16.0f / 9.0f; // width / height
constexpr int IMAGE_HEIGHT = (int)(IMAGE_WIDTH / ASPECT);
constexpr RenderType RENDER_TYPE = RenderType::All;
constexpr bool SIMPLE_RENDER = (RENDER_TYPE != RenderType::Light) && (RENDER_TYPE != RenderType::Samples) && (RENDER_TYPE != RenderType::All);
constexpr unsigned MIN_DEPTH   = SIMPLE_RENDER ? 1  : 3; // minimum bounces
constexpr unsigned MAX_DEPTH   = SIMPLE_RENDER ? 1  : 5; // max bounced to prevent too much recursion
constexpr unsigned MIN_SAMPLES = SIMPLE_RENDER ? 1  : 4; // minimum adaptive samples
constexpr unsigned MAX_SAMPLES = SIMPLE_RENDER ? 1  : 8; // cutoff for adaptive sampling
constexpr float SAMPLE_THRESHOLD = 0.01f; // threshold for dynamic sampling
constexpr unsigned FOV = 70; // field of view
constexpr int BILATERAL_RADIUS = 0; // radius of bilateral filter blur

constexpr float EXPOSURE = 1.0f;

constexpr float NEAR_PLANE = 3.0f;
constexpr float FAR_PLANE = 20.0f;
