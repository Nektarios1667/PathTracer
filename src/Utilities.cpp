#include "Utilities.h"
#include "Sphere.h"
#include <random>
#include <sstream>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <string>
#include <unordered_map>

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

    std::vector<std::unique_ptr<Hittable>> readObjFile(const std::string& filename) {
        std::vector<Vector3> vertices;
        std::vector<std::unique_ptr<Hittable>> triangles;
        std::shared_ptr<Material> meshMaterial = std::make_shared<Material>(Color(1.0f, 0.4f, 0.4f), Color(), 0.0f, 1.0f);

        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            } else if (prefix == "f") {
                int i1, i2, i3;
                iss >> i1 >> i2 >> i3;
                triangles.emplace_back(std::make_unique<Triangle>(vertices[i1 - 1], vertices[i2 - 1], vertices[i3 - 1], meshMaterial));
            }
        }
        return triangles;
    }
    void PTSParseError(const std::string& message, const std::string& line, int lineNumber) {
        cerr << "PTS Parsing Error - " << message << " at line " << to_string(lineNumber) << ": '" << line << "'\n";
    }

    SceneSetup readPtsFile(const std::string& filename) {
        SceneSetup scene;

        // Setup streams
        std::ifstream filereader(filename);
        std::string line;

        // Iterate lines
        std::unordered_map<string, std::shared_ptr<Material>> materials;
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
                        PTSParseError("Expected 'Vector3'", line, l);
                    scene.cameraFrom = from;
                } else {
                    cerr << "Unknown PTS key 'from:' in scope '" + scope + "'\n";
                }
            } else if (prefix == "to:") {
                if (scope == "camera") {
                    Vector3 to;
                    if (!(linereader >> to))
                        PTSParseError("Expected 'Vector3'", line, l);
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
                        PTSParseError("Expected 'string Color Color float float'", line, l);
                    materials[name] = std::make_shared<Material>(albedo, emission, refl, rough);
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
                        PTSParseError("Expected 'Vector3 Vector3 Vector3 Material'", line, l);
                    if (materials.find(matString) == materials.end())
                        PTSParseError("Could not find PTS defined Material '" + matString + "'", line, l);
                    scene.hittables.push_back(std::make_unique<Triangle>(v0, v1, v2, materials[matString]));
                } else {
                    cerr << "Unknown PTS key 'triangle:' in scope '" + scope + "'\n";
                }
            } else if (prefix == "sphere:") {
                if (scope == "objects") {
                    Vector3 center;
                    float radius;
                    string matString;

                    if (!(linereader >> center >> radius >> matString))
                        PTSParseError("Expected 'Vector3 float Material'", line, l);
                    if (materials.find(matString) == materials.end())
                        PTSParseError("Could not find PTS defined Material '" + matString + "'", line, l);
                    scene.hittables.push_back(std::make_unique<Sphere>(center, radius, materials[matString]));
                }
            }
            // Read
            else if (prefix == "obj:") {
                if (scope == "read") {
                    std::string file;
                    if (!(linereader >> file))
                        PTSParseError("Expected 'string'", line, l);

                    auto loadedScene = readObjFile(file);
                    scene.hittables.insert(scene.hittables.end(), std::make_move_iterator(loadedScene.begin()), std::make_move_iterator(loadedScene.end()));
                } else {
                    cerr << "Unknown PTS key 'obj:' in scope '" + scope + "'\n";
                }
            }
            l++;
        }

        return scene;
    }
}
