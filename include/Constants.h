#pragma once

constexpr unsigned MIN_DEPTH = 4; // minimum bounces
constexpr unsigned MAX_DEPTH = 8; // max bounced to prevent too much recursion
constexpr unsigned MIN_SAMPLES = 8; // minimum adaptive samples
constexpr unsigned MAX_SAMPLES = 64; // cutoff for adaptive sampling
constexpr float SAMPLE_THRESHOLD = 0.001f; // threshold for dynamic sampling
constexpr unsigned FOV = 70; // field of view
constexpr float ASPECT = 16.0f / 9.0f; // width / height