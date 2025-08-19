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
#include "lodepng.h"

#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Light.h"
#include "Utilities.h"
#include "Constants.h"
#include "BVHNode.h"
#include "BVHStats.h"
#include <mutex>

using namespace std;
using namespace std::chrono;

// Atomic counter
std::atomic<int> tileCounter;
std::mutex coutMutex;

constexpr int TILE_SIZE = 16;
void renderTile(Camera& camera, std::unique_ptr<BVHNode>& rootBVH, vector<PixelData>& pixelBuffer, int width, int height, int tilesX, int tilesY) {
    while (true) {
        int index = tileCounter.fetch_add(1);
        if (index >= tilesX * tilesY) {
            cout << "\rThread: " << index << "/" << index << " (100%)";
            break;
        }

        // Print progress
        if (index % std::max(1, tilesX * tilesY / 100) == 0 && coutMutex.try_lock()) {
            cout << "\rThread: " << index << "/" << tilesX * tilesY << " (" << std::round((index / (float)(tilesX * tilesY)) * 100) << "%)";
            coutMutex.unlock();
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

    // Settings
    int imageWidth = IMAGE_WIDTH;
    int imageHeight = IMAGE_HEIGHT;

    // Print
    const std::string settings =
        "Settings:\n"
        "  FOV: " + std::to_string(FOV) + "\n"
        "  Width: " + std::to_string(imageWidth) + "\n"
        "  Height: " + std::to_string(imageHeight) + "\n"
        "  Sampling: " + std::to_string(MIN_SAMPLES) + "-" + std::to_string(MAX_SAMPLES) + "\n"
        "  Depth: " + std::to_string(MIN_DEPTH) + "-" + std::to_string(MAX_DEPTH) + "\n"
        "  Threshold: " + std::to_string(SAMPLE_THRESHOLD) + "\n"
        "  Bilateral Radius: " + to_string(BILATERAL_RADIUS) + "\n";
    cout << settings << endl;

    // Object read
    SceneSetup setup = Utilities::readTrcFile("data/TestScene.trc");
    cout << "Creating BVH...\n";

    // Camera
    Camera camera(setup.cameraFrom, setup.cameraTo, Vector3(0, 1, 0), FOV, ASPECT);

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
    vector<unsigned char> pixels(imageWidth * imageHeight * 4);
    vector<PixelData> pixelDataBuffer(imageWidth * imageHeight);

    // Calculate number of tiles
    int tilesX = (imageWidth + TILE_SIZE - 1) / TILE_SIZE;
    int tilesY = (imageHeight + TILE_SIZE - 1) / TILE_SIZE;

    // Ray tracing
    // Launch threads equal to logical cores
    tileCounter = 0;
    constexpr int THREADS = 4;
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back(renderTile, std::ref(camera), std::ref(rootBVH), std::ref(pixelDataBuffer), imageWidth, imageHeight, tilesX, tilesY);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();  // blocks until this thread completes
    }

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
    startTime = high_resolution_clock::now();
    if (RENDER_TYPE == RenderType::All) {
        // Light
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Light);
        lodepng::encode("render.png", pixels, imageWidth, imageHeight);
        // Normals
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Normals);
        lodepng::encode("render_normals.png", pixels, imageWidth, imageHeight);
        // Depth
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Depth);
        lodepng::encode("render_depth.png", pixels, imageWidth, imageHeight);
        // BVH
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::BVH);
        lodepng::encode("render_bvh.png", pixels, imageWidth, imageHeight);
        // Samples
        pixels = camera.getRenderOutput(pixelDataBuffer, RenderType::Samples);
        lodepng::encode("render_samples.png", pixels, imageWidth, imageHeight);
    }
    else {
        pixels = camera.getRenderOutput(pixelDataBuffer, RENDER_TYPE);
        lodepng::encode("render_" + RenderTypeMap.at(RENDER_TYPE) + ".png", pixels, imageWidth, imageHeight);
    }
    ofstream metadata("metadata.txt");
    metadata << "Rendered with C++ path tracer made by Nektarios.\n" + settings + "\n" + bvhString + "\nCompleted in " + to_string(renderDuration) + "s (" + to_string(renderDuration / 60) + " m)";

    // Print file stats 
    endTime = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(endTime - startTime).count();
    cout << "Completed write in " << duration << " ms." << endl;

    return 0;
}