#include <iostream>
#include <chrono>
#include <cmath>
#include <fstream>
#include <cfloat>
#include <vector>
#include <memory>
#include <random>
#include <sstream>
#include <omp.h>
#include <string>
#include <thread>
#include <conio.h>
#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>
#include <filesystem>
#pragma comment(lib, "winmm.lib")
#include "lodepng.h"

#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Utilities.h"
#include "Constants.h"
#include "BVHNode.h"
#include "BVHStats.h"
#include <mutex>

using namespace std;
using namespace std::chrono;

// Atomic counter
std::atomic<int> tileCounter;
std::atomic<bool> cancelRender(false);

void OutputRenderSnapshot(const Camera camera, const std::vector<PixelData>& pixelDataBuffer, std::chrono::steady_clock::time_point startTime, const std::string settings, const std::string bvhString) {
    // Print render stats
    auto endTime = high_resolution_clock::now();
    int renderDuration = duration_cast<seconds>(endTime - startTime).count();
    cout << "\nRender snapshot at " << renderDuration << " s." << endl;

    // Write to file
	std::filesystem::create_directory(OUTPUT_DIR + "snapshot");
    vector<unsigned char> pixels(IMAGE_WIDTH * IMAGE_HEIGHT * 4);
    startTime = high_resolution_clock::now();
    if (RENDER_TYPE == RenderType::All) {
        // Light
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Light);
        lodepng::encode(OUTPUT_DIR + "snapshot/render.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Normals
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Normals);
        lodepng::encode(OUTPUT_DIR + "snapshot/render_normals.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Depth
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Depth);
        lodepng::encode(OUTPUT_DIR + "snapshot/render_depth.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // BVH
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::BVH);
        lodepng::encode(OUTPUT_DIR + "snapshot/render_bvh.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Samples
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Samples);
        lodepng::encode(OUTPUT_DIR + "snapshot/render_samples.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    else {
        pixels = camera.getRenderOutput(pixelDataBuffer, RENDER_TYPE);
        lodepng::encode(OUTPUT_DIR + "snapshot/render_" + RenderTypeMap.at(RENDER_TYPE) + ".png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    }

    ofstream metadata(OUTPUT_DIR + "snapshot/metadata.txt");
    metadata << "Rendered with C++ path tracer made by Nektarios.\n[v" + VERSION + "]\n" + settings + "\n" + bvhString + "\Snapshot at " + to_string(renderDuration) + "s (" + to_string(renderDuration / 60) + " m)";
}

void OutputFinalRender(const Camera camera, vector<PixelData>& pixelDataBuffer, std::chrono::steady_clock::time_point startTime, const std::string settings, const std::string bvhString) {
    // Print render stats
    auto endTime = high_resolution_clock::now();
    int renderDuration = duration_cast<seconds>(endTime - startTime).count();
    cout << "\nCompleted render in " << renderDuration << " s.\nPassing though post processing..." << endl;

    // Post process bilateral filter
    startTime = high_resolution_clock::now();
    if (BILATERAL_RADIUS > 0) {
        vector<PixelData> temp(pixelDataBuffer.size());
        camera.bilateralBlurHorizontal(pixelDataBuffer, temp);
        camera.bilateralBlurVertical(temp, pixelDataBuffer);
    }

    endTime = high_resolution_clock::now();
    int duration = duration_cast<seconds>(endTime - startTime).count();
    cout << "Completed post processing in " << duration << " s.\nWriting to file..." << endl;

    // Write to file
    vector<unsigned char> pixels(IMAGE_WIDTH * IMAGE_HEIGHT * 4);
    startTime = high_resolution_clock::now();
    if (RENDER_TYPE == RenderType::All) {
        // Light
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Light);
        lodepng::encode(OUTPUT_DIR + "render.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Normals
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Normals);
        lodepng::encode(OUTPUT_DIR + "render_normals.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Depth
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Depth);
        lodepng::encode(OUTPUT_DIR + "render_depth.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // BVH
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::BVH);
        lodepng::encode(OUTPUT_DIR + "render_bvh.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        // Samples
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Samples);
        lodepng::encode(OUTPUT_DIR + "render_samples.png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    else {
        pixels = camera.getRenderOutput(pixelDataBuffer, RENDER_TYPE);
        lodepng::encode(OUTPUT_DIR + "render_" + RenderTypeMap.at(RENDER_TYPE) + ".png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    ofstream metadata(OUTPUT_DIR + "metadata.txt");
    metadata << "Rendered with C++ path tracer made by Nektarios.\n[v" + VERSION + "]\n" + settings + "\n" + bvhString + "\nCompleted in " + to_string(renderDuration) + "s (" + to_string(renderDuration / 60) + " m)";

    // Print file stats 
    endTime = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(endTime - startTime).count();
    cout << "Completed write in " << duration << " ms." << endl;
}

constexpr int TILE_SIZE = 16;
void renderTile(Camera& camera, std::unique_ptr<BVHNode>& rootBVH, vector<PixelData>& pixelBuffer, int width, int height, int tilesX, int tilesY) {
    while (!cancelRender) {
        int index = tileCounter.fetch_add(1, std::memory_order_relaxed);
        if (index >= tilesX * tilesY) {
            break;
        }

        // Calculate pixels in tile
        int tileX = (index % tilesX) * TILE_SIZE;
        int tileY = (index / tilesX) * TILE_SIZE;

        for (int y = tileY; y < std::min(tileY + TILE_SIZE, height); ++y) {
            for (int x = tileX; x < std::min(tileX + TILE_SIZE, width); ++x) {
                pixelBuffer[y * width + x] = camera.tracePixel(x, y, width, height, rootBVH.get());
            }
        }
    }
}

int main() {
    auto startTime = high_resolution_clock::now();
	std::filesystem::create_directory(OUTPUT_DIR);

    // Print
    const std::string settings =
        "Settings:\n"
        "  FOV: " + std::to_string(FOV) + "\n"
        "  Width: " + std::to_string(IMAGE_WIDTH) + "\n"
        "  Height: " + std::to_string(IMAGE_HEIGHT) + "\n"
        "  Sampling: " + std::to_string(MIN_SAMPLES) + "-" + std::to_string(MAX_SAMPLES) + "\n"
        "  Depth: " + std::to_string(MIN_DEPTH) + "-" + std::to_string(MAX_DEPTH) + "\n"
        "  Threshold: " + std::to_string(SAMPLE_THRESHOLD) + "\n"
        "  Bilateral Radius: " + to_string(BILATERAL_RADIUS) + "\n";
	cout << "[v" + VERSION + "]\n";
    cout << settings << endl;

    // Object read
    SceneSetup setup = Utilities::readTrcFile("data/TestScene.trc");
    cout << "Creating BVH...\n";

    // Camera
    Camera camera(setup.cameraFrom, setup.cameraTo, Vector3d(0.0, 1.0, 0.0), FOV, ASPECT);

    // Hittables
    BVHNode::scene = std::move(setup.hittables);

    // Raw scene
    vector<Hittable*> rawScene;
    rawScene.reserve(BVHNode::scene.size());
    for (auto& hittable : BVHNode::scene) {
        rawScene.push_back(hittable.get());
    }
    unique_ptr<BVHNode> rootBVH = make_unique<BVHNode>(rawScene, 0, rawScene.size());

    // Debug
    BVHStats stats;
    BVHNode::getNodeDebugInfo(rootBVH.get(), 1, stats);

    const std::string bvhString = 
    "BVH Stats:\n"
    "  Total nodes: " + std::to_string(stats.totalNodes) + "\n"
    "  Max depth: " + std::to_string(stats.maxDepth) + "\n"
    "  Total leaf nodes: " + std::to_string(stats.totalLeafNodes) + "\n";
    cout << bvhString << endl;

    // Output 
    vector<PixelData> pixelDataBuffer(IMAGE_WIDTH * IMAGE_HEIGHT);

    // Calculate number of tiles
    int tilesX = (IMAGE_WIDTH + TILE_SIZE - 1) / TILE_SIZE;
    int tilesY = (IMAGE_HEIGHT + TILE_SIZE - 1) / TILE_SIZE;

    // Ray tracing
    // Launch threads equal to logical cores
    tileCounter = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back(renderTile, std::ref(camera), std::ref(rootBVH), std::ref(pixelDataBuffer), IMAGE_WIDTH, IMAGE_HEIGHT, tilesX, tilesY);
    }

    // Progress
	int totalTiles = tilesX * tilesY;
    auto lastSnapshot = high_resolution_clock::now();
    auto lastPercent = 0;
    while (tileCounter < totalTiles && !cancelRender) {
        // Progress
        int done = tileCounter.load(std::memory_order_relaxed);
        float percent = (done / (float)(totalTiles)) * 100.0f;
        cout << "\rRendering: " << std::round(percent) << "% (" << done << "/" << totalTiles << ")" << std::flush;
        
        // Snapshot every so often
        if (high_resolution_clock::now() - lastSnapshot >= minutes(10) || percent - lastPercent >= 20) {
            lastSnapshot = high_resolution_clock::now();
			lastPercent = percent;
			OutputRenderSnapshot(camera, pixelDataBuffer, startTime, settings, bvhString);
		}

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();  // blocks until this thread completes
    }

    // Output
	OutputFinalRender(camera, pixelDataBuffer, startTime, settings, bvhString);

    return 0;
}