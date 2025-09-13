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
#include <mutex>
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
#include "TraceParser.h"

using namespace std;
using namespace std::chrono;

// Atomic counter
std::atomic<int> tileCounter;
std::atomic<bool> cancelRender(false);

static void writeRenderOutputs(const Camera& camera, const std::vector<PixelData>& buffer, const std::string& prefix) {
    std::vector<unsigned char> pixels(IMAGE_WIDTH * IMAGE_HEIGHT * 4);

    if (RENDER_TYPE == RenderType::All) {
        for (RenderType type : { RenderType::Light, RenderType::Normals, RenderType::Depth, RenderType::BVH, RenderType::Samples }) {
            pixels = camera.getRenderOutput(buffer, type);
            lodepng::encode(OUTPUT_DIR + prefix + RenderTypeMap.at(type) + ".png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
        }
    }
    else {
        pixels = camera.getRenderOutput(buffer, RENDER_TYPE);
        lodepng::encode(OUTPUT_DIR + prefix + RenderTypeMap.at(RENDER_TYPE) + ".png", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
    }
}

static void writeMetadata(const std::string& filepath, int renderDuration, const std::string& settings, const std::string& bvhString, const std::string& label) {
    std::ofstream metadata(filepath);
    metadata << "Rendered with C++ path tracer made by Nektarios.\n[v" << VERSION << "]\n"
        << settings << "\n"
        << bvhString << "\n"
        << label << " at " << renderDuration << "s (" << renderDuration / 60 << " m)";
}

static void outputRenderSnapshot(const Camera& camera, const std::vector<PixelData>& pixelDataBuffer, std::chrono::steady_clock::time_point startTime, const std::string& settings, const std::string& bvhString) {
    using namespace std::chrono;
    int renderDuration = duration_cast<seconds>(high_resolution_clock::now() - startTime).count();
    std::cout << "\nRender snapshot at " << renderDuration << " s." << std::endl;

    std::filesystem::create_directory(OUTPUT_DIR + "snapshot");
    writeRenderOutputs(camera, pixelDataBuffer, "snapshot/render_");
    writeMetadata(OUTPUT_DIR + "snapshot/metadata.txt", renderDuration, settings, bvhString, "Snapshot");
}

static void outputFinalRender(const Camera& camera, std::vector<PixelData>& pixelDataBuffer, std::chrono::steady_clock::time_point startTime, const std::string& settings, const std::string& bvhString) {
    using namespace std::chrono;
    int renderDuration = duration_cast<seconds>(high_resolution_clock::now() - startTime).count();
    std::cout << "\nCompleted render in " << renderDuration << " s.\nPassing though post processing..." << std::endl;

    // Post process (placeholder)
    auto postStart = high_resolution_clock::now();
    // ...
    auto duration = duration_cast<seconds>(high_resolution_clock::now() - postStart).count();
    std::cout << "Completed post processing in " << duration << " s.\nWriting to file..." << std::endl;

    auto writeStart = high_resolution_clock::now();
    writeRenderOutputs(camera, pixelDataBuffer, "render_");
    writeMetadata(OUTPUT_DIR + "metadata.txt", renderDuration, settings, bvhString, "Completed");

    int writeMs = duration_cast<milliseconds>(high_resolution_clock::now() - writeStart).count();
    std::cout << "Completed write in " << writeMs << " ms." << std::endl;
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
    const std::string settings = Utilities::getMetadata();
    cout << settings << endl;

    // Object read
    SceneSetup setup = TraceParser::readTrcFile(SCENE);
    cout << "Creating BVH...\n";

    // Camera
    Camera camera(setup.cameraFrom, setup.cameraTo, Vector3d(0.0, 1.0, 0.0), setup.fov, ASPECT);

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
    auto lastPercent = 0.0f;
    while (tileCounter < totalTiles && !cancelRender) {
        // Progress
        int done = tileCounter.load(std::memory_order_relaxed);
        float percent = (done / (float)(totalTiles)) * 100.0f;
        cout << "\rRendering: " << std::round(percent) << "% (" << done << "/" << totalTiles << ")" << std::flush;
        
        // Snapshot every so often
        if (high_resolution_clock::now() - lastSnapshot >= minutes(20) || percent - lastPercent >= 20) {
            lastSnapshot = high_resolution_clock::now();
			lastPercent = percent;
			outputRenderSnapshot(camera, pixelDataBuffer, startTime, settings, bvhString);
		}

        // Wait
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Output
	outputFinalRender(camera, pixelDataBuffer, startTime, settings, bvhString);

    return 0;
}