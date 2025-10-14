#pragma once

#include "ScriptProperty.h"
#include <filesystem>
#include <vector>
#include <string>

namespace Supernova::Editor {

    class ScriptParser {
    private:
        static Supernova::ScriptPropertyType inferTypeFromCppType(const std::string& cppType, std::string& ptrTypeName);
        static Supernova::ScriptPropertyType parseExplicitType(const std::string& typeStr, const std::string& cppType);
        static std::string removeComments(const std::string& content);

    public:
        static std::vector<Supernova::ScriptProperty> parseScriptProperties(const std::filesystem::path& scriptPath);
    };

}