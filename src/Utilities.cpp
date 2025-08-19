#include "Utilities.h"
#include "Sphere.h"
#include <random>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Utilities {
    float randomFloat() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }

    Color randomColor() {
        return Color(randomFloat(), randomFloat(), randomFloat());
    }

    Vector3 randomInUnitSphere() {
        while (true) {
            Vector3 p(randomFloat() * 2 - 1, randomFloat() * 2 - 1, randomFloat() * 2 - 1);
            if (p.lengthSquared() < 1) return p;
        }
    }

    Vector3 randomCosineHemisphere(const Vector3& normal) {
        double r1 = 2.0 * Utilities::PI * ((double)rand() / RAND_MAX);
        double r2 = (double)rand() / RAND_MAX;
        double r = sqrt(r2);
        double z = sqrt(1.0 - r2);

        Vector3 u = ((fabs(normal.x) > 0.1) ? Vector3(0, 1, 0) : Vector3(1, 0, 0)).cross(normal).normalized();
        Vector3 v = normal.cross(u);

        return (u * (cos(r1) * r) + v * (sin(r1) * r) + normal * z).normalized();
    }

    std::vector<std::unique_ptr<Hittable>> readObjFile(const std::string& filename, std::shared_ptr<Material> meshMaterial, float scale, Vector3 offset) {
        std::vector<Vector3> vertices;
        std::vector<std::unique_ptr<Hittable>> triangles;

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x * scale + offset.x, y * scale + offset.y, z * scale + offset.z);
            } else if (prefix == "f") {
                int i1, i2, i3;
                iss >> i1 >> i2 >> i3;
                triangles.emplace_back(std::make_unique<Triangle>(vertices[i1 - 1], vertices[i2 - 1], vertices[i3 - 1], meshMaterial));
            }
        }
        return triangles;
    }
    void TRCParseError(const std::string& message, const std::string& line, int lineNumber) {
        cerr << "PTS Parsing Error - " << message << " at line " << to_string(lineNumber) << ": '" << line << "'\n";
    }
    SceneSetup readTrcFile(const std::string& filename, std::vector<string> layers) {
        SceneSetup scene;
        layers.push_back(filename);

        // Setup streams
        std::ifstream filereader(filename);
        std::string line;

        // Iterate lines
        std::string scope = "";
        int l = 1;
        while (std::getline(filereader, line)) {
            // Read line
            std::istringstream linereader(line);
            std::string prefix;
            
            // Parse
            linereader >> prefix;
            if (prefix.empty() || prefix == "//") {
                // Nothing
            // Scopes
            } else if (prefix == "[camera]") {
                scope = "camera";
            } else if (prefix == "[materials]") {
                scope = "materials";
            } else if (prefix == "[objects]") {
                scope = "objects";
            } else if (prefix == "[read]") {
                scope = "read";
            } else if (prefix.front() == '[' && prefix.back() == ']') {
                cerr << "Unknown PTS scope '" + prefix + "'";
            // Keys
            // Camera
            } else if (prefix == "from:") {
                if (scope == "camera") {
                    Vector3 from;
                    if (!(linereader >> from))
                        TRCParseError("Expected 'Vector3'", line, l);
                    scene.cameraFrom = from;
                } else {
                    cerr << "Unknown PTS key 'from:' in scope '" + scope + "'\n";
                }
            } else if (prefix == "to:") {
                if (scope == "camera") {
                    Vector3 to;
                    if (!(linereader >> to))
                        TRCParseError("Expected 'Vector3'", line, l);
                    scene.cameraTo = to;
                } else {
                    cerr << "Unknown PTS key 'to:' in scope '" + scope + "'\n";
                }
            // Materials
            } else if (prefix == "mat:") {
                if (scope == "materials") {
                    std::string name;
                    Color albedo, emission;
                    float refl, rough;
                    
                    if (!(linereader >> name >> albedo >> emission >> refl >> rough))
                        TRCParseError("Expected 'string Color Color float float'", line, l);
                    scene.materials[name] = std::make_shared<Material>(albedo, emission, refl, rough);
                } else {
                    cerr << "Unknown PTS key 'mat:' in scope '" + scope + "'\n";
                }
            }
            // Objects
            else if (prefix == "triangle:") {
                if (scope == "objects") {
                    Vector3 v0, v1, v2;
                    string matString;

                    if (!(linereader >> v0 >> v1 >> v2 >> matString))
                        TRCParseError("Expected 'Vector3 Vector3 Vector3 Material'", line, l);
                    if (scene.materials.find(matString) == scene.materials.end())
                        TRCParseError("Could not find PTS defined Material '" + matString + "'", line, l);
                    scene.hittables.push_back(std::make_unique<Triangle>(v0, v1, v2, scene.materials[matString]));
                } else {
                    cerr << "Unknown PTS key 'triangle:' in scope '" + scope + "'\n";
                }
            } else if (prefix == "sphere:") {
                if (scope == "objects") {
                    Vector3 center;
                    float radius;
                    std::string matString;

                    if (!(linereader >> center >> radius >> matString))
                        TRCParseError("Expected 'Vector3 float Material'", line, l);
                    if (scene.materials.find(matString) == scene.materials.end())
                        TRCParseError("Could not find PTS defined Material '" + matString + "'", line, l);
                    scene.hittables.push_back(std::make_unique<Sphere>(center, radius, scene.materials[matString]));
                }
            }
            // Read
            else if (prefix == "obj:") {
                if (scope == "read") {
                    std::string file;
                    string matString;
                    float scale;
                    Vector3 offset;

                    if (!(linereader >> file >> scale >> offset >> matString))
                        TRCParseError("Expected 'string float Vector3 Material'", line, l);
                    if (scene.materials.find(matString) == scene.materials.end())
                        TRCParseError("Could not find PTS defined Material '" + matString + "'", line, l);
                    auto loadedScene = readObjFile(file, scene.materials[matString], scale, offset);
                    scene.hittables.insert(scene.hittables.end(), std::make_move_iterator(loadedScene.begin()), std::make_move_iterator(loadedScene.end()));
                } else {
                    cerr << "Unknown PTS key 'obj:' in scope '" + scope + "'\n";
                }
            }
            else if (prefix == "trc:") {
                if (scope == "read") {
                    std::string file;
                    string matString;

                    if (!(linereader >> file))
                        TRCParseError("Expected 'string'", line, l);
                    if (std::find(layers.begin(), layers.end(), file) != layers.end())
                        TRCParseError("Infinite recursion found loading TRC '" + file + "'", line, l);
                    auto loadedScene = readTrcFile(file, layers);
                    // Objects
                    scene.hittables.insert(scene.hittables.end(), std::make_move_iterator(loadedScene.hittables.begin()), std::make_move_iterator(loadedScene.hittables.end()));
                    // Materials
                    scene.materials.insert(loadedScene.materials.begin(), loadedScene.materials.end());
                }
                else {
                    cerr << "Unknown PTS key 'trc:' in scope '" + scope + "'\n";
                }
            }
            l++;
        }

        return scene;
    }
}
