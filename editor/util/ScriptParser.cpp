#include "ScriptParser.h"
#include "Out.h"
#include <fstream>
#include <regex>
#include <sstream>
#include <algorithm>
#include <unordered_set>

using namespace Supernova;


ScriptPropertyType Editor::ScriptParser::inferTypeFromCppType(const std::string& cppType) {
    // Remove const, &, *, and whitespace for comparison
    std::string cleanType = cppType;
    cleanType.erase(std::remove_if(cleanType.begin(), cleanType.end(), ::isspace), cleanType.end());

    // Remove const qualifier
    size_t constPos = cleanType.find("const");
    if (constPos != std::string::npos) {
        cleanType.erase(constPos, 5);
    }

    // Remove references and pointers
    cleanType.erase(std::remove(cleanType.begin(), cleanType.end(), '&'), cleanType.end());
    cleanType.erase(std::remove(cleanType.begin(), cleanType.end(), '*'), cleanType.end());

    // Map C++ types to ScriptPropertyType
    if (cleanType == "bool") {
        return ScriptPropertyType::Bool;
    }

    if (cleanType == "int" || cleanType == "int32_t" || cleanType == "uint32_t" ||
        cleanType == "short" || cleanType == "long") {
        return ScriptPropertyType::Int;
    }

    if (cleanType == "float" || cleanType == "double") {
        return ScriptPropertyType::Float;
    }

    if (cleanType == "std::string" || cleanType == "string") {
        return ScriptPropertyType::String;
    }

    if (cleanType == "Vector2" || cleanType == "Supernova::Vector2") {
        return ScriptPropertyType::Vector2;
    }

    if (cleanType == "Vector3" || cleanType == "Supernova::Vector3") {
        return ScriptPropertyType::Vector3;
    }

    if (cleanType == "Vector4" || cleanType == "Supernova::Vector4") {
        return ScriptPropertyType::Vector4;
    }

    // Default to int if unknown
    Out::warning("Unknown C++ type '%s', defaulting to Int", cppType.c_str());
    return ScriptPropertyType::Int;
}

std::string Editor::ScriptParser::removeComments(const std::string& content) {
    std::string result;
    result.reserve(content.size());

    enum class State { Normal, InLineComment, InBlockComment, InString, InChar };
    State state = State::Normal;

    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        char next = (i + 1 < content.size()) ? content[i + 1] : '\0';

        switch (state) {
            case State::Normal:
                if (c == '/' && next == '/') {
                    state = State::InLineComment;
                    i++; // Skip next char
                } else if (c == '/' && next == '*') {
                    state = State::InBlockComment;
                    i++; // Skip next char
                } else if (c == '"') {
                    state = State::InString;
                    result += c;
                } else if (c == '\'') {
                    state = State::InChar;
                    result += c;
                } else {
                    result += c;
                }
                break;

            case State::InLineComment:
                if (c == '\n') {
                    state = State::Normal;
                    result += c; // Keep newline
                }
                break;

            case State::InBlockComment:
                if (c == '*' && next == '/') {
                    state = State::Normal;
                    i++; // Skip next char
                }
                break;

            case State::InString:
                result += c;
                if (c == '\\' && next != '\0') {
                    result += next;
                    i++; // Skip escaped char
                } else if (c == '"') {
                    state = State::Normal;
                }
                break;

            case State::InChar:
                result += c;
                if (c == '\\' && next != '\0') {
                    result += next;
                    i++; // Skip escaped char
                } else if (c == '\'') {
                    state = State::Normal;
                }
                break;
        }
    }

    return result;
}

std::vector<ScriptProperty> Editor::ScriptParser::parseScriptProperties(const std::filesystem::path& scriptPath) {
    std::vector<ScriptProperty> properties;

    if (!std::filesystem::exists(scriptPath)) {
        return properties;
    }

    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        Out::error("Failed to open script file for parsing: %s", scriptPath.string().c_str());
        return properties;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    content = removeComments(content);

    // Pattern: SPROPERTY("Display Name") Type varName = defaultValue; or SPROPERTY("Display Name") Type varName;
    std::regex propertyRegex(
        "SPROPERTY\\s*\\(\\s*"           // SPROPERTY(
        "\"([^\"]+)\"\\s*"                // "Display Name"
        "\\)\\s*"                         // )
        "([\\w:]+(?:\\s*<[^>]+>)?)\\s+"  // Type (with templates)
        "(\\w+)\\s*"                      // varName
        "(?:=\\s*([^;]+?))?\\s*;"        // optional = defaultValue
    );

    std::sregex_iterator it(content.begin(), content.end(), propertyRegex);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        size_t position = match.position();

        std::string displayName = match[1].str();
        std::string cppType = match[2].str();
        std::string varName = match[3].str();
        std::string defaultValueStr = match[4].str(); // May be empty

        // Remove whitespace from type
        cppType.erase(std::remove_if(cppType.begin(), cppType.end(), ::isspace), cppType.end());

        // Remove whitespace from default value if present
        if (!defaultValueStr.empty()) {
            defaultValueStr.erase(std::remove_if(defaultValueStr.begin(), defaultValueStr.end(), ::isspace), defaultValueStr.end());
        }

        // Infer ScriptPropertyType from C++ type
        ScriptPropertyType type = inferTypeFromCppType(cppType);

        ScriptProperty prop;
        prop.name = varName;
        prop.displayName = displayName;
        prop.type = type;

        // Parse and set default value based on type
        try {
            switch (type) {
                case ScriptPropertyType::Bool: {
                    bool val = bool(); // default value
                    if (!defaultValueStr.empty()) {
                        val = (defaultValueStr == "true" || defaultValueStr == "1");
                    }
                    prop.value.boolValue = val;
                    prop.defaultValue.boolValue = val;
                    break;
                }

                case ScriptPropertyType::Int: {
                    int val = int(); // default value
                    if (!defaultValueStr.empty()) {
                        val = std::stoi(defaultValueStr);
                    }
                    prop.value.intValue = val;
                    prop.defaultValue.intValue = val;
                    break;
                }

                case ScriptPropertyType::Float: {
                    float val = float(); // default value
                    if (!defaultValueStr.empty()) {
                        // Handle 'f' suffix
                        std::string cleanFloat = defaultValueStr;
                        if (!cleanFloat.empty() && cleanFloat.back() == 'f') {
                            cleanFloat.pop_back();
                        }
                        val = std::stof(cleanFloat);
                    }
                    prop.value.floatValue = val;
                    prop.defaultValue.floatValue = val;
                    break;
                }

                case ScriptPropertyType::String: {
                    std::string val = std::string(); // default value
                    if (!defaultValueStr.empty()) {
                        // Remove quotes if present
                        val = defaultValueStr;
                        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                            val = val.substr(1, val.size() - 2);
                        }
                    }
                    prop.value.stringValue = val;
                    prop.defaultValue.stringValue = val;
                    break;
                }

                case ScriptPropertyType::Vector2: {
                    Vector2 val = Vector2(); // default value
                    if (!defaultValueStr.empty()) {
                        // Parse Vector2(x, y) or Supernova::Vector2(x, y)
                        std::regex vecRegex("(?:Supernova::)?Vector2\\(([^,]+),([^)]+)\\)");
                        std::smatch vecMatch;
                        if (std::regex_search(defaultValueStr, vecMatch, vecRegex)) {
                            std::string xStr = vecMatch[1].str();
                            std::string yStr = vecMatch[2].str();
                            // Remove 'f' suffix if present
                            if (!xStr.empty() && xStr.back() == 'f') xStr.pop_back();
                            if (!yStr.empty() && yStr.back() == 'f') yStr.pop_back();
                            float x = std::stof(xStr);
                            float y = std::stof(yStr);
                            val = Vector2(x, y);
                        }
                    }
                    prop.value.vector2Value = val;
                    prop.defaultValue.vector2Value = val;
                    break;
                }

                case ScriptPropertyType::Vector3:
                case ScriptPropertyType::Color3: {
                    Vector3 val = Vector3(); // default value
                    if (!defaultValueStr.empty()) {
                        // Parse Vector3(x, y, z) or Supernova::Vector3(x, y, z)
                        std::regex vecRegex("(?:Supernova::)?Vector3\\(([^,]+),([^,]+),([^)]+)\\)");
                        std::smatch vecMatch;
                        if (std::regex_search(defaultValueStr, vecMatch, vecRegex)) {
                            std::string xStr = vecMatch[1].str();
                            std::string yStr = vecMatch[2].str();
                            std::string zStr = vecMatch[3].str();
                            // Remove 'f' suffix if present
                            if (!xStr.empty() && xStr.back() == 'f') xStr.pop_back();
                            if (!yStr.empty() && yStr.back() == 'f') yStr.pop_back();
                            if (!zStr.empty() && zStr.back() == 'f') zStr.pop_back();
                            float x = std::stof(xStr);
                            float y = std::stof(yStr);
                            float z = std::stof(zStr);
                            val = Vector3(x, y, z);
                        }
                    }
                    prop.value.vector3Value = val;
                    prop.defaultValue.vector3Value = val;
                    break;
                }

                case ScriptPropertyType::Vector4:
                case ScriptPropertyType::Color4: {
                    Vector4 val = Vector4(); // default value
                    if (!defaultValueStr.empty()) {
                        // Parse Vector4(x, y, z, w) or Supernova::Vector4(x, y, z, w)
                        std::regex vecRegex("(?:Supernova::)?Vector4\\(([^,]+),([^,]+),([^,]+),([^)]+)\\)");
                        std::smatch vecMatch;
                        if (std::regex_search(defaultValueStr, vecMatch, vecRegex)) {
                            std::string xStr = vecMatch[1].str();
                            std::string yStr = vecMatch[2].str();
                            std::string zStr = vecMatch[3].str();
                            std::string wStr = vecMatch[4].str();
                            // Remove 'f' suffix if present
                            if (!xStr.empty() && xStr.back() == 'f') xStr.pop_back();
                            if (!yStr.empty() && yStr.back() == 'f') yStr.pop_back();
                            if (!zStr.empty() && zStr.back() == 'f') zStr.pop_back();
                            if (!wStr.empty() && wStr.back() == 'f') wStr.pop_back();
                            float x = std::stof(xStr);
                            float y = std::stof(yStr);
                            float z = std::stof(zStr);
                            float w = std::stof(wStr);
                            val = Vector4(x, y, z, w);
                        }
                    }
                    prop.value.vector4Value = val;
                    prop.defaultValue.vector4Value = val;
                    break;
                }
            }
        } catch (const std::exception& e) {
            size_t lineNum = std::count(content.begin(), content.begin() + position, '\n') + 1;
            Out::warning("Failed to parse property at line %zu: %s", lineNum, e.what());
            // Keep zero/empty defaults on parse error
        }

        properties.push_back(prop);
    }

    std::unordered_set<std::string> seenNames;
    for (const auto& prop : properties) {
        if (!seenNames.insert(prop.name).second) {
            Out::warning("Duplicate property name '%s' in %s", 
                         prop.name.c_str(), scriptPath.string().c_str());
        }
    }

    return properties;
}
