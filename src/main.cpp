#include <iostream>
#include <chrono>
#include <cmath>
#include <fstream>
#include <windows.h>
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
        "  Bilateral Radius: " + to_string(BILATERAL_RADIUS);
    cout << settings << endl;

    // Camera
    Vector3 from = Vector3(0, 1.5f, 2.5f);
    Vector3 to = Vector3(0, 0, 0);
    Camera camera(from, to, Vector3(0, 1, 0), FOV, ASPECT);

    // Object read
    BVHNode::scene = Utilities::readObjFile("C:/Users/nekta/Downloads/MonkeySubdivided.obj");
    // Ground plane
    BVHNode::scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(0, -1000.5f, -5), 1000.0f, { Color(1.0f, 0.2f, 0.2f), Color(), 0.0f, 1.0f } }));
    // Light source
    BVHNode::scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(0, 5, -1), 3.0f, { Color(), Color(0.0f, 1.0f, 0.0f), 0.0f, 1.0f } }));
    // Shiny round thingy
    BVHNode::scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(-2.5f, 0.5f, 0), 0.75f, Material{ Color(0.0f, 1.0f, 0.5f), Color(), 0.0f, 1.0f }}));
    // Kinda shiny round thingy
    BVHNode::scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(2.5f, 0.5f, 0), 0.75f, Material{ Color(0.62f, 0.51f, 0.93f), Color(), 1.0f, 0.5f }}));
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

    std::cout << "Total nodes: " << stats.totalNodes << "\n";
    std::cout << "Max depth: " << stats.maxDepth << "\n";
    std::cout << "Total leaf nodes: " << stats.totalLeafNodes << "\n";

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
        if (y % (imageHeight / 33) == 0 || y == imageHeight - 1) {
            float fraction = float(y) / (imageHeight - 1);
            int percentage = round(fraction * 100);
            string bar = "";
            for (int i = 0; i < 33; i++) {
                bar += i < percentage / 3 ? "▮" : "▯";
            }
            cout << "\rProgress: " << percentage << "% " << bar << flush;
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

    // ShellExecuteA(nullptr, "open", "output.png", nullptr, nullptr, SW_SHOWNORMAL);

    return 0;
}