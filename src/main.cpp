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

using namespace std;
using namespace std::chrono;


int main() {
    auto startTime = high_resolution_clock::now();

    // Settings
    int imageWidth = IMAGE_WIDTH;
    int imageHeight = IMAGE_HEIGHT;

    // Print
    const string settings =
        "Settings:\n"
        "  FOV: " + std::to_string(FOV) + "\n"
        "  Width: " + std::to_string(imageWidth) + "\n"
        "  Height: " + std::to_string(imageHeight) + "\n"
        "  Sampling: " + std::to_string(MIN_SAMPLES) + "-" + std::to_string(MAX_SAMPLES) + "\n"
        "  Depth: " + std::to_string(MIN_DEPTH) + "-" + std::to_string(MAX_DEPTH) + "\n"
        "  Threshold: " + std::to_string(SAMPLE_THRESHOLD) + "\n"
        "  Bilateral Radius: " + to_string(BILATERAL_RADIUS) + "\n";
    cout << settings << endl;

    // Materials
    std::shared_ptr<Material> groundMat = std::make_shared<Material>(Color(1.0f, 0.2f, 0.2f), Color(), 0.0f, 1.0f);
    std::shared_ptr<Material> whiteLight = std::make_shared<Material>(Color(), Color(1.0f), 0.0f, 0.0f);

    // Object read
    SceneSetup setup = Utilities::readPtsFile("data/TestScene.trc");

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

	std::cout << "BVH Stats:\n";
    std::cout << "  Total nodes: " << stats.totalNodes << "\n";
    std::cout << "  Max depth: " << stats.maxDepth << "\n";
    std::cout << "  Total leaf nodes: " << stats.totalLeafNodes << "\n";
    std::cout << endl;

    // Output 
    vector<unsigned char> pixels(imageWidth * imageHeight * 4);
    vector<PixelData> pixelDataBuffer(imageWidth * imageHeight);

    // Ray tracing
    #pragma omp parallel for
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            // Data
            PixelData pixel = camera.tracePixel(x, y, imageWidth, imageHeight, rootBVH);
            pixelDataBuffer[y * imageWidth + x] = pixel;
        }

        // Progress bar
        if (y % (imageHeight / 33) == 0 || y == imageHeight - 1 || y == 0) {
            float fraction = float(y) / (imageHeight - 1);
            int percentage = round(fraction * 100);
            string bar = "";
            for (int i = 0; i < 33; i++) {
                bar += i < percentage / 3 ? "#" : "-";
            }
            cout << "\rProgress: " << percentage << "% [" << bar << "]";
        }
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
    pixels = camera.getRenderOutput(pixelDataBuffer);

    endTime = high_resolution_clock::now();
    int duration = duration_cast<seconds>(endTime - startTime).count();
    cout << "Completed post processing in " << duration << " s.\nWriting to file..." << endl;

    // Write to file5
    startTime = high_resolution_clock::now();
    lodepng::encode("output.png", pixels, imageWidth, imageHeight);
    ofstream metadata("metadata.txt");
    metadata << "Rendered with C++ path tracer made by Nektarios.\n" + settings + "\nCompleted in " + to_string(renderDuration) + "s (" + to_string(renderDuration / 60) + " m)";

    // Print file stats 
    endTime = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(endTime - startTime).count();
    cout << "Completed write in " << duration << " ms." << endl;

    return 0;
}