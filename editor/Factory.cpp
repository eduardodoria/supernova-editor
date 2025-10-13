#include "Factory.h"

#include "Scene.h"
#include "Catalog.h"
#include <sstream>
#include <iomanip>

using namespace Supernova;

Editor::Factory::Factory(){
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
        case PropertyType::Float:
        case PropertyType::Float_0_1: {
            float* value = static_cast<float*>(property.ref);
            return formatFloat(*value);
        }
        case PropertyType::Int:
        case PropertyType::IntSlider: {
            int* value = static_cast<int*>(property.ref);
            return formatInt(*value);
        }
        case PropertyType::UInt:
        case PropertyType::UIntSlider: {
            unsigned int* value = static_cast<unsigned int*>(property.ref);
            return formatUInt(*value);
        }
        case PropertyType::Vector2: {
            Vector2* value = static_cast<Vector2*>(property.ref);
            return formatVector2(*value);
        }
        case PropertyType::Vector3:
        case PropertyType::Color3L:
        case PropertyType::Direction: {
            Vector3* value = static_cast<Vector3*>(property.ref);
            return formatVector3(*value);
        }
        case PropertyType::Vector4:
        case PropertyType::Color4L: {
            Vector4* value = static_cast<Vector4*>(property.ref);
            return formatVector4(*value);
        }
        case PropertyType::Quat: {
            Quaternion* value = static_cast<Quaternion*>(property.ref);
            return formatQuaternion(*value);
        }
        case PropertyType::String:
        case PropertyType::Script: {
            std::string* value = static_cast<std::string*>(property.ref);
            return formatString(*value);
        }
        case PropertyType::Enum: {
            int* value = static_cast<int*>(property.ref);
            // Find enum name if available
            if (property.enumEntries) {
                for (const auto& entry : *property.enumEntries) {
                    if (entry.value == *value) {
                        return "/* " + std::string(entry.name) + " */ " + formatInt(*value);
                    }
                }
            }
            return formatInt(*value);
        }
        case PropertyType::HalfCone: {
            // HalfCone is typically two floats (inner and outer angles)
            float* value = static_cast<float*>(property.ref);
            return formatFloat(*value);
        }
        case PropertyType::Material:
        case PropertyType::Texture:
        default:
            return "/* unsupported property type */";
    }
}

std::string Editor::Factory::createComponent(Scene* scene, Entity entity, ComponentType componentType) {
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

    code << "    " << componentName << " " << varName << ";\n";

    // Generate code for each property
    for (const auto& [propertyName, propertyData] : properties) {
        std::string formattedValue = formatPropertyValue(propertyData, propertyName);
        code << "    " << varName << "." << propertyName << " = " << formattedValue << ";\n";
    }

    return code.str();
}

std::string Editor::Factory::createAllComponents(Scene* scene, Entity entity) {
    // Find all components for this entity
    std::vector<ComponentType> components = Catalog::findComponents(scene, entity);

    std::ostringstream code;
    code << "// Entity components initialization\n";

    for (ComponentType componentType : components) {
        std::string componentCode = createComponent(scene, entity, componentType);
        if (!componentCode.empty()) {
            code << componentCode << "\n";
        }
    }

    return code.str();
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