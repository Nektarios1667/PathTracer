#pragma once
#include <map>
#include <thread>
#include <string>

enum class Resolution {
    VeryLow = 320,
    Low = 640,
    Medium = 1280,
    High = 1920,
    VeryHigh = 3840,
    Ultra = 7680,
};

enum class RenderType {
    Light,   // Photo render
	Normals, // Normal vectors
	Depth,   // Depth map
	BVH,     // BVH visualization
	Samples, // Sample count
	All,     // Render all types
};
const std::map<RenderType, std::string> RenderTypeMap = {
    {RenderType::Light, "light"},
    {RenderType::Normals, "normals"},
    {RenderType::Depth, "depth"},
    {RenderType::BVH, "bvh"},
    {RenderType::Samples, "samples"},
    {RenderType::All, "all"},
};

// Render settings
const std::string SCENE = "data/Cornell.trc";
constexpr RenderType RENDER_TYPE = RenderType::All;
constexpr bool SIMPLE_RENDER = (RENDER_TYPE != RenderType::Light) && (RENDER_TYPE != RenderType::Samples) && (RENDER_TYPE != RenderType::All);

constexpr int IMAGE_WIDTH = (int)Resolution::Low;
constexpr float ASPECT = 16.0f / 9.0f; // width / height

constexpr unsigned MIN_DEPTH   = SIMPLE_RENDER ? 0 : 3; // minimum bounces
constexpr unsigned MAX_DEPTH   = SIMPLE_RENDER ? 0 : 5; // max bounced to prevent too much recursion
constexpr unsigned MIN_SAMPLES = SIMPLE_RENDER ? 1 : 8; // minimum adaptive samples
constexpr unsigned MAX_SAMPLES = SIMPLE_RENDER ? 1 : 32; // cutoff for adaptive sampling
constexpr float SAMPLE_THRESHOLD = .0001f; // threshold for dynamic sampling

// Automatic
constexpr int IMAGE_HEIGHT = (int)(IMAGE_WIDTH / ASPECT);

// Technical
constexpr unsigned THREADS = 4;
const std::string VERSION = "2.2.0";

// Post processing
constexpr float EXPOSURE = 1.0f;
constexpr bool TONE_MAPPED = true; // apply tone mapping
const std::string OUTPUT_DIR = "output/"; // output directory

// Debug
constexpr float NEAR_PLANE = 3.0f;
constexpr float FAR_PLANE = 20.0f;
