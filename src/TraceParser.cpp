#include "TraceParser.h"
#include <algorithm>

ssMap TraceParser::globalVariables;

// Key handlers
static std::string handleCameraFrom(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "camera")
        return "Key 'from:' should be in scope 'camera'";

    Vector3 from;
    if (!(linereader >> from))
        return "Expected 'Vector3'";
    scene.cameraFrom = from;
    return "";
}

static std::string handleCameraTo(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "camera")
        return "Key 'from:' should be in scope 'camera'";

    Vector3 to;
    if (!(linereader >> to))
        return "Expected 'Vector3'";
    scene.cameraTo = to;

    return "";
}

static std::string handleMaterialMat(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "materials")
        return "Key 'mat:' should be in scope 'materials'";

    std::string name;
    Color albedo, emission;
    float refl, rough, refract;

    if (!(linereader >> name >> albedo >> emission >> refl >> rough >> refract))
        return "Expected 'string Color Color float float float'";
    scene.materials[name] = std::make_shared<Material>(albedo, emission, refl, rough, refract);

    return "";
}

static std::string handleObjectsTriangle(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "objects")
        return "Key 'triangle:' should be in scope 'objects'";

    Vector3 v0, v1, v2;
    string matString;

    if (!(linereader >> v0 >> v1 >> v2 >> matString))
        return "Expected 'Vector3 Vector3 Vector3 Material'";
    if (scene.materials.find(matString) == scene.materials.end())
        return "Could not find PTS defined Material '" + matString + "'";
    scene.hittables.push_back(std::make_unique<Triangle>(v0, v1, v2, scene.materials[matString]));

    return "";
}

static std::string handleObjectsSphere(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "objects")
        return "Key 'sphere:' should be in scope 'objects'";

    Vector3 center;
    float radius;
    std::string matString;

    if (!(linereader >> center >> radius >> matString))
        return "Expected 'Vector3 float Material'";
    if (scene.materials.find(matString) == scene.materials.end())
        return "Could not find PTS defined Material '" + matString + "'";
    scene.hittables.push_back(std::make_unique<Sphere>(center, radius, scene.materials[matString]));

    return "";
}

static std::string handleReadObj(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "read")
        return "Key 'obj:' should be in scope 'read'";

    std::string file, matString;
    float scale;
    Vector3 offset;

    if (!(linereader >> file >> scale >> offset >> matString))
        return "Expected 'string float Vector3 Material'";
    if (scene.materials.find(matString) == scene.materials.end())
        return "Could not find PTS defined Material '" + matString + "'";
    auto loadedScene = Utilities::readObjFile(file, scene.materials[matString], scale, offset);
    scene.hittables.insert(scene.hittables.end(), std::make_move_iterator(loadedScene.begin()), std::make_move_iterator(loadedScene.end()));

    return "";
}

static std::string handleReadTrc(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "read")
        return "Key 'trc:' should be in scope 'read'";

    std::string file;
    string matString;

    if (!(linereader >> file))
        return "Expected 'string'";
    auto loadedScene = TraceParser::readTrcFile(file);
    // Objects
    scene.hittables.insert(scene.hittables.end(), std::make_move_iterator(loadedScene.hittables.begin()), std::make_move_iterator(loadedScene.hittables.end()));
    // Materials
    scene.materials.insert(loadedScene.materials.begin(), loadedScene.materials.end());

    return "";
}

static std::string handleVariablesLocal(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "variables")
        return "Key 'local:' should be in scope 'variables'";

    std::string name, value;
    if (!(linereader >> name >> value))
        return "Expected 'string string'";
    localVariables[name] = value;

    return "";
}

static std::string handleVariablesGlobal(const std::string& scope, std::istringstream& linereader, SceneSetup& scene, ssMap& localVariables, ssMap& globalVariables) {
    if (scope != "variables")
        return "Key 'global:' should be in scope 'variables'";

    std::string name, value;
    if (!(linereader >> name >> value))
        return "Expected 'string string'";
    globalVariables[name] = value;

    return "";
}

// Handlers register
const std::vector <TraceKey> TraceParser::traceKeys = {
        TraceKey("from:", "camera", handleCameraFrom),
        TraceKey("to:", "camera", handleCameraTo),
        TraceKey("mat:", "materials", handleMaterialMat),
        TraceKey("triangle:", "objects", handleObjectsTriangle),
        TraceKey("sphere:", "objects", handleObjectsSphere),
        TraceKey("obj:", "read", handleReadObj),
        TraceKey("trc:", "read", handleReadTrc),
        TraceKey("local:", "variables", handleVariablesLocal),
        TraceKey("global:", "variables", handleVariablesGlobal),
        // Add other keys and their handlers here
};

std::string TraceParser::injectVariables(std::string text, ssMap localVariables) {    
    for (const auto& [key, value] : localVariables) {
        std::string findKey = "=" + key;

        int start = text.find(findKey);
        while (start != std::string::npos) {
            text = text.replace(start, findKey.length(), value);
            start = text.find(findKey);
        }
    }

    for (const auto& [key, value] : TraceParser::globalVariables) {
        std::string findKey = "=" + key;

        int start = text.find(findKey);
        while (start != std::string::npos) {
            text = text.replace(start, findKey.length(), value);
            start = text.find(findKey);
        }
    }

    return text;
}

SceneSetup TraceParser::readTrcFile(const std::string& filename) {
    SceneSetup scene;

    // Setup
    std::ifstream filereader(filename);
    std::string line;
    std::string scope = "";
    ssMap localVariables;
    if (!filereader)
        throw std::runtime_error("Unable to open file: " + filename);

    // Iterate lines
    std::vector<std::string> ifSkips;
    int l = 1;
    while (std::getline(filereader, line)) {
        // Replace variables
        line = injectVariables(line, localVariables);

        // Read line
        std::istringstream linereader(line);
        std::string prefix;

        // Parse
        linereader >> prefix;
        if (prefix.empty() || prefix == "//")
            continue;
        
        // Flow
        if (prefix.front() == '.') {
            if (prefix == ".if") {
                bool condition;
                std::string branchName;
                if (!(linereader >> condition >> branchName))
                    throwParseError("Expected '.if bool string'", line, l, filename);
                else if (condition)
                    ifSkips.push_back(branchName);
            }
            else if (prefix == ".endif") {
                std::string branchName;
                if (!(linereader >> branchName))
                    throwParseError("Expected '.endif string'", line, l, filename);
                else {
                    auto idx = std::find(ifSkips.begin(), ifSkips.end(), branchName);
                    if (idx != ifSkips.end())
                        ifSkips.erase(idx);
                }
            }
        }

        // Scopes
        bool foundScope = false;
        if (prefix.front() == '[' && prefix.back() == ']') {
            for (const auto& s : SCOPES) {
                if (prefix == "[" + std::string(s) + "]") {
                    scope = s;
                    foundScope = true;
                    break;
                }
            }
            if (!foundScope)
                throwParseError("Unknown PTS scope '" + prefix + "'", line, l, filename);
            continue;
        }

        // Get key
        bool foundKey = false;
        for (const auto& tk : traceKeys) {
            if (prefix == tk.key) {
                std::string resp = tk.parser(scope, linereader, scene, localVariables, globalVariables);
                foundKey = true;
                if (!resp.empty())
                    throwParseError(resp, line, l, filename);
                break;
            }
        }
        if (!foundKey)
            throwParseError("Unknown PTS key '" + prefix + "'", line, l, filename);

        l++;
    }

    return scene;
}