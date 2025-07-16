#pragma once

constexpr unsigned MAX_DEPTH = 2; // 0 = no reflections
constexpr unsigned MIN_SAMPLES = 4; // dynamic sampling min
constexpr unsigned MAX_SAMPLES = 128; // dynamic sampling max
constexpr float SAMPLE_THRESHOLD = 0.005f;
constexpr unsigned FOV = 70; // field of view
constexpr float ASPECT = 16.0f / 9.0f; // width / height