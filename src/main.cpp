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
#include "Light.h"
#include "Utilities.h"
#include "Constants.h"

using namespace std;
using namespace std::chrono;


int main() {
    auto startTime = high_resolution_clock::now();

    // Settings
    int imageWidth = RESOLUTION;
    int imageHeight = IMAGE_HEIGHT;

    // Print
    const string settings =
        "Settings:\n"
        "  FOV: " + std::to_string(FOV) + "\n"
        "  Width: " + std::to_string(imageWidth) + "\n"
        "  Height: " + std::to_string(imageHeight) + "\n"
        "  Sampling: " + std::to_string(MIN_SAMPLES) + "-" + std::to_string(MAX_SAMPLES) + "\n"
        "  Depth: " + std::to_string(MIN_DEPTH) + "-" + std::to_string(MAX_DEPTH) + "\n"
        "  Threshold: " + std::to_string(SAMPLE_THRESHOLD);
    cout << settings << endl;

    // Lighting
    Light ambientLight = { Vector3(), Color(1, 1, 1), .1f };
    // NOTE Normalize light directions for correct calculations
    vector<Light> lights = {
        { Vector3(0, 5, 0), Color(0, 1, 1), 15.0f },
        { Vector3(-2, 2, 1), Color(1, 0, 1), 10.0f },
    };

    // Camera
    Vector3 from = Vector3(0, 1, 3);
    Vector3 to = Vector3(0, 0, -1);
    Camera camera(from, to, Vector3(0, 1, 0), FOV, ASPECT);

    // Ground plane
    vector<unique_ptr<Hittable>> scene;
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(0, -1000.5f, -1), 1000.0f, { Color(0.8f), Color(), 0.0f, 1.0f } }));
    // Colored spheres
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(0, 0, -3), 0.5f, { Color(1.0f, 0.1f, 0.1f), Color(), 0.0f, 1.0f } })); // red matte
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(-1, 0, -2.5), 0.5f, { Color(0.1f, 1.0f, 0.1f), Color(), 0.8f, 0.6f } })); // green metal
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(1, 0, -4), 0.5f, { Color(0.8f), Color(), 1.0f, 0.0f } })); // gray mirror
    // Light source (bright emissive)
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(0, 2, -1), 0.5f, { Color(), Color(5.0f), 0.0f, 1.0f } }));
    scene.push_back(std::make_unique<Sphere>(Sphere{ Vector3(3, 0, -3), 0.5f, { Color(1.0f, 1.0f, 0.2f), Color(), 0.0f, 1.0f } })); // yellow wall

    // Output
    vector<unsigned char> pixels(imageWidth * imageHeight * 4);
    vector<PixelData> pixelDataBuffer(imageWidth * imageHeight);

    // vector<float> depth(imageWidth * imageHeight);
    // vector<Vector3> normals(imageWidth * imageHeight);

    // Ray tracing
    #pragma omp parallel for
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            // Data
            PixelData pixel = camera.tracePixel(x, y, imageWidth, imageHeight, scene);
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
    auto duration = duration_cast<seconds>(endTime - startTime).count();
    cout << "\nCompleted render in " << duration << " s.\nPassing though post processing..." << endl;

    // Post process bilateral filter
    startTime = high_resolution_clock::now();
    // vector<PixelData> temp(pixelDataBuffer.size());
    // camera.bilateralBlurHorizontal(pixelDataBuffer, temp);
    // camera.bilateralBlurVertical(temp, pixelDataBuffer);
    pixels = camera.getRenderOutput(pixelDataBuffer);

    endTime = high_resolution_clock::now();
    duration = duration_cast<seconds>(endTime - startTime).count();
    cout << "Completed post processing in " << duration << " s.\nWriting to file..." << endl;

    // Write to file5
    startTime = high_resolution_clock::now();
    lodepng::encode("output.png", pixels, imageWidth, imageHeight);
    ofstream metadata("metadata.txt");
    metadata << "Rendered with C++ path tracer made by Nektarios.\n" + settings + "\nCompleted in " + to_string(duration) + "s (" + to_string(duration / 60) + " m)";

    // Print file stats 
    endTime = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(endTime - startTime).count();
    cout << "Completed write in " << duration << " ms." << endl;

    ShellExecuteA(nullptr, "open", "output.png", nullptr, nullptr, SW_SHOWNORMAL);

    return 0;
}