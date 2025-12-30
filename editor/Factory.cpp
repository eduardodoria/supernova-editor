#include "Factory.h"

#include "Scene.h"
#include "Catalog.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <unordered_set>

using namespace Supernova;

Editor::Factory::Factory(){
}

std::string Editor::Factory::indentation(int spaces) {
    if (spaces <= 0) {
        return "";
    }
    return std::string(static_cast<size_t>(spaces), ' ');
}

std::string Editor::Factory::toIdentifier(const std::string& name) {
    std::string out;
    out.reserve(name.size());

    auto isIdentStart = [](unsigned char c) {
        return std::isalpha(c) || c == '_';
    };
    auto isIdentChar = [](unsigned char c) {
        return std::isalnum(c) || c == '_';
    };

    bool lastWasUnderscore = false;
    for (unsigned char c : name) {
        if (isIdentChar(c)) {
            out.push_back(static_cast<char>(c));
            lastWasUnderscore = (c == '_');
        } else {
            if (!out.empty() && !lastWasUnderscore) {
                out.push_back('_');
                lastWasUnderscore = true;
            }
        }
    }

    while (!out.empty() && out.front() == '_') {
        out.erase(out.begin());
    }
    while (!out.empty() && out.back() == '_') {
        out.pop_back();
    }

    if (out.empty()) {
        out = "var";
    }

    if (!isIdentStart(static_cast<unsigned char>(out[0]))) {
        out.insert(out.begin(), '_');
    }

    static const std::unordered_set<std::string> kCppKeywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
        "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
        "class", "compl", "concept", "const", "consteval", "constexpr", "constinit",
        "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
        "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit",
        "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline",
        "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "register",
        "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static",
        "static_assert", "static_cast", "struct", "switch", "template", "this", "thread_local",
        "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
        "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };

    if (kCppKeywords.find(out) != kCppKeywords.end()) {
        out.push_back('_');
    }

    return out;
}

std::string Editor::Factory::formatVector2(const Vector2& v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "Vector2(" << v.x << "f, " << v.y << "f)";
    return oss.str();
}

std::string Editor::Factory::formatVector3(const Vector3& v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "Vector3(" << v.x << "f, " << v.y << "f, " << v.z << "f)";
    return oss.str();
}

std::string Editor::Factory::formatVector4(const Vector4& v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "Vector4(" << v.x << "f, " << v.y << "f, " << v.z << "f, " << v.w << "f)";
    return oss.str();
}

std::string Editor::Factory::formatQuaternion(const Quaternion& q) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "Quaternion(" << q.w << "f, " << q.x << "f, " << q.y << "f, " << q.z << "f)";
    return oss.str();
}

std::string Editor::Factory::formatBool(bool value) {
    return value ? "true" : "false";
}

std::string Editor::Factory::formatFloat(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << value << "f";
    return oss.str();
}

std::string Editor::Factory::formatInt(int value) {
    return std::to_string(value);
}

std::string Editor::Factory::formatUInt(unsigned int value) {
    return std::to_string(value);
}

std::string Editor::Factory::formatString(const std::string& value) {
    return "\"" + value + "\"";
}

std::string Editor::Factory::formatPropertyValue(const PropertyData& property, const std::string& propertyName) {
    switch (property.type) {
        case PropertyType::Bool: {
            bool* value = static_cast<bool*>(property.ref);
            return formatBool(*value);
        }
        case PropertyType::Float: {
            float* value = static_cast<float*>(property.ref);
            return formatFloat(*value);
        }
        case PropertyType::Int: {
            int* value = static_cast<int*>(property.ref);
            return formatInt(*value);
        }
        case PropertyType::UInt: {
            unsigned int* value = static_cast<unsigned int*>(property.ref);
            return formatUInt(*value);
        }
        case PropertyType::Vector2: {
            Vector2* value = static_cast<Vector2*>(property.ref);
            return formatVector2(*value);
        }
        case PropertyType::Vector3: {
            Vector3* value = static_cast<Vector3*>(property.ref);
            return formatVector3(*value);
        }
        case PropertyType::Vector4: {
            Vector4* value = static_cast<Vector4*>(property.ref);
            return formatVector4(*value);
        }
        case PropertyType::Quat: {
            Quaternion* value = static_cast<Quaternion*>(property.ref);
            return formatQuaternion(*value);
        }
        case PropertyType::String: {
            std::string* value = static_cast<std::string*>(property.ref);
            return formatString(*value);
        }
        case PropertyType::Enum: {
            int* value = static_cast<int*>(property.ref);
            return formatInt(*value);
        }
        case PropertyType::Material:
        case PropertyType::Texture:
        default:
            return "/* unsupported property type */";
    }
}

std::string Editor::Factory::createComponent(int indentSpaces, Scene* scene, Entity entity, ComponentType componentType, std::string sceneName) {
    // Get all properties for this component
    std::map<std::string, PropertyData> properties = Catalog::findEntityProperties(scene, entity, componentType);

    if (properties.empty()) {
        return "";
    }

    std::ostringstream code;
    std::string componentName = Catalog::getComponentName(componentType, false);
    std::string varName = Catalog::getComponentName(componentType, true);
    const std::string ind = indentation(indentSpaces);

    // Convert first letter to lowercase for variable name
    if (!varName.empty()) {
        varName[0] = std::tolower(varName[0]);
    }

    code << ind << componentName << " " << varName << ";\n";

    // Generate code for each property
    for (const auto& [propertyName, propertyData] : properties) {
        std::string formattedValue = formatPropertyValue(propertyData, propertyName);
        code << ind << varName << "." << propertyName << " = " << formattedValue << ";\n";
    }

    if (!sceneName.empty()) {
        code << ind << sceneName << ".addComponent<" << componentName << ">(" << entity << ", " << varName << ");\n";
    }

    return code.str();
}

std::string Editor::Factory::createAllComponents(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    // Find all components for this entity
    std::vector<ComponentType> components = Catalog::findComponents(scene, entity);

    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "// Entity components initialization\n";

    for (ComponentType componentType : components) {
        std::string componentCode = createComponent(indentSpaces, scene, entity, componentType, sceneName);
        if (!componentCode.empty()) {
            code << componentCode << "\n";
        }
    }

    return code.str();
}

std::string Editor::Factory::createScene(int indentSpaces, Scene* scene, std::string name, std::vector<Entity> entities){
    std::ostringstream out;

    std::string mainSceneVar = toIdentifier(name);
    const std::string ind = indentation(indentSpaces);

    out << ind << "#include \"Supernova.h\"\n";
    out << ind << "using namespace Supernova;\n\n";
    out << ind << "Scene " << mainSceneVar << ";\n";
    //out << ind << "Scene " << childSceneVar << ";\n\n";
    out << ind << "void init(){\n\n";

    const std::string ind2 = indentation(indentSpaces+4);

    for (Entity entity : entities) {
        out << ind2 << "// Entity " << entity << "\n";
        out << ind2 << mainSceneVar << ".recreateEntity(" << entity << ");\n\n";

        // Create and set all components
        std::string componentsCode = createAllComponents(indentSpaces+4, scene, entity, mainSceneVar);
        out << componentsCode;

        out << "\n";
    }

    //out << "\n";
    out << ind2 << "Engine::setCanvasSize(" << "1000, 480);\n";
    //out << "\n";
    out << ind2 << "Engine::setScene(&" << mainSceneVar << ");\n";
    //out << ind2 << "    Engine::addSceneLayer(&" << childSceneVar << ");\n";
    out << ind << "}\n";

    return out.str();
}

std::string Editor::Factory::setComponent(Scene* scene, Entity entity, ComponentType componentType) {
    // Check if entity has this component
    Signature signature = scene->getSignature(entity);
    ComponentId compId = Catalog::getComponentId(scene, componentType);

    if (!signature.test(compId)) {
        return "";
    }

    // Get all properties for this component
    std::map<std::string, PropertyData> properties = Catalog::findEntityProperties(scene, entity, componentType);

    if (properties.empty()) {
        return "";
    }

    std::ostringstream code;
    std::string componentName = Catalog::getComponentName(componentType, false);
    std::string varName = Catalog::getComponentName(componentType, true);

    // Convert first letter to lowercase for variable name
    if (!varName.empty()) {
        varName[0] = std::tolower(varName[0]);
    }

    code << "    if (signature.test(scene->getComponentId<" << componentName << ">())) {\n";
    code << "        " << componentName << "& " << varName << " = scene->getComponent<" << componentName << ">(entity);\n";

    // Generate code for each property
    for (const auto& [propertyName, propertyData] : properties) {
        std::string formattedValue = formatPropertyValue(propertyData, propertyName);
        code << "        " << varName << "." << propertyName << " = " << formattedValue << ";\n";
    }

    code << "    }\n";

    return code.str();
}

std::string Editor::Factory::setAllComponents(Scene* scene, Entity entity) {
    // Find all components for this entity
    std::vector<ComponentType> components = Catalog::findComponents(scene, entity);

    std::ostringstream code;
    code << "    // Set entity components\n";
    code << "    Signature signature = scene->getSignature(entity);\n";
    code << "\n";

    for (ComponentType componentType : components) {
        std::string componentCode = setComponent(scene, entity, componentType);
        if (!componentCode.empty()) {
            code << componentCode << "\n";
        }
    }

    return code.str();
}