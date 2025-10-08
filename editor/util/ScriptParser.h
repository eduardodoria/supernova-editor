#pragma once

#include "ScriptProperty.h"
#include <filesystem>
#include <vector>
#include <string>

namespace Supernova::Editor {

class ScriptParser {
public:
    static std::vector<Supernova::ScriptProperty> parseScriptProperties(const std::filesystem::path& scriptPath);

private:
    static Supernova::ScriptPropertyType inferTypeFromCppType(const std::string& cppType);
};

}