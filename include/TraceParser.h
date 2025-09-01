#pragma once
#include "Vector3.h"
#include "Camera.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <sstream>
#include <iostream>
#include <Triangle.h>
#include <Sphere.h>
#include <Utilities.h>

using ssMap = std::unordered_map<std::string, std::string>;
using traceKeyParseFunc = std::function<std::string(const std::string&, std::istringstream&, SceneSetup&, ssMap&, ssMap&)>;

class TraceKey {
public:
    std::string_view key;
    std::string_view scope;
    traceKeyParseFunc parser;

    TraceKey(std::string_view key, std::string_view scope, const traceKeyParseFunc& func) : key(key), scope(scope), parser(func) {}
};

class TraceParser {
private:
    static ssMap globalVariables;

    static inline const std::string_view SCOPES[5] = { "variables", "camera", "materials", "objects", "read" };
    static const std::vector<TraceKey> traceKeys;

public:
    static std::string injectVariables(std::string text, ssMap localVariables);
	static void throwParseError(const std::string& message, const std::string& line, int lineNumber, const std::string& file) {
		cerr << "PTS Parsing Error in '" << file << "' - " << message << " at line " << to_string(lineNumber) << ": '" << line << "'\n";
	}
    static SceneSetup readTrcFile(const std::string& filename);
};