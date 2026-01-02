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

std::string Editor::Factory::formatRect(const Rect& r) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "Rect(" << r.getX() << "f, " << r.getY() << "f, " << r.getWidth() << "f, " << r.getHeight() << "f)";
    return oss.str();
}

std::string Editor::Factory::formatPrimitiveType(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::TRIANGLES: return "PrimitiveType::TRIANGLES";
        case PrimitiveType::TRIANGLE_STRIP: return "PrimitiveType::TRIANGLE_STRIP";
        case PrimitiveType::POINTS: return "PrimitiveType::POINTS";
        case PrimitiveType::LINES: return "PrimitiveType::LINES";
        default: return "PrimitiveType::TRIANGLES";
    }
}

std::string Editor::Factory::formatPivotPreset(PivotPreset preset) {
    switch (preset) {
        case PivotPreset::CENTER: return "PivotPreset::CENTER";
        case PivotPreset::TOP_CENTER: return "PivotPreset::TOP_CENTER";
        case PivotPreset::BOTTOM_CENTER: return "PivotPreset::BOTTOM_CENTER";
        case PivotPreset::LEFT_CENTER: return "PivotPreset::LEFT_CENTER";
        case PivotPreset::RIGHT_CENTER: return "PivotPreset::RIGHT_CENTER";
        case PivotPreset::TOP_LEFT: return "PivotPreset::TOP_LEFT";
        case PivotPreset::BOTTOM_LEFT: return "PivotPreset::BOTTOM_LEFT";
        case PivotPreset::TOP_RIGHT: return "PivotPreset::TOP_RIGHT";
        case PivotPreset::BOTTOM_RIGHT: return "PivotPreset::BOTTOM_RIGHT";
        default: return "PivotPreset::BOTTOM_LEFT";
    }
}

std::string Editor::Factory::formatLightType(LightType type) {
    switch (type) {
        case LightType::DIRECTIONAL: return "LightType::DIRECTIONAL";
        case LightType::POINT: return "LightType::POINT";
        case LightType::SPOT: return "LightType::SPOT";
        default: return "LightType::DIRECTIONAL";
    }
}

std::string Editor::Factory::formatScriptType(ScriptType type) {
    switch (type) {
        case ScriptType::SUBCLASS: return "ScriptType::SUBCLASS";
        case ScriptType::SCRIPT_CLASS: return "ScriptType::SCRIPT_CLASS";
        case ScriptType::SCRIPT_LUA: return "ScriptType::SCRIPT_LUA";
        default: return "ScriptType::SUBCLASS";
    }
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

std::string Editor::Factory::createTransform(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<Transform>(entity)) return "";
    Transform& transform = scene->getComponent<Transform>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "Transform transform;\n";
    code << ind << "transform.position = " << formatVector3(transform.position) << ";\n";
    code << ind << "transform.rotation = " << formatQuaternion(transform.rotation) << ";\n";
    code << ind << "transform.scale = " << formatVector3(transform.scale) << ";\n";
    code << ind << "transform.visible = " << formatBool(transform.visible) << ";\n";
    code << ind << "transform.billboard = " << formatBool(transform.billboard) << ";\n";
    code << ind << "transform.fakeBillboard = " << formatBool(transform.fakeBillboard) << ";\n";
    code << ind << "transform.cylindricalBillboard = " << formatBool(transform.cylindricalBillboard) << ";\n";
    code << ind << "transform.billboardRotation = " << formatQuaternion(transform.billboardRotation) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<Transform>(" << entity << ", transform);\n";
        else
            code << ind << sceneName << ".addComponent<Transform>(" << entity << ", transform);\n";
    }
    return code.str();
}

std::string Editor::Factory::createMeshComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<MeshComponent>(entity)) return "";
    MeshComponent& mesh = scene->getComponent<MeshComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "MeshComponent mesh;\n";
    code << ind << "mesh.castShadows = " << formatBool(mesh.castShadows) << ";\n";
    code << ind << "mesh.receiveShadows = " << formatBool(mesh.receiveShadows) << ";\n";
    //code << ind << "mesh.submeshes.resize(" << formatUInt(mesh.numSubmeshes) << ");\n";
    code << ind << "mesh.numSubmeshes = " << formatUInt(mesh.numSubmeshes) << ";\n";
    for (unsigned int s = 0; s < mesh.numSubmeshes; s++){
        std::string idx = std::to_string(s);
        code << ind << "mesh.submeshes[" << idx << "].material.name = " << formatString(mesh.submeshes[s].material.name) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].material.baseColorFactor = " << formatVector4(mesh.submeshes[s].material.baseColorFactor) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].material.metallicFactor = " << formatFloat(mesh.submeshes[s].material.metallicFactor) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].material.roughnessFactor = " << formatFloat(mesh.submeshes[s].material.roughnessFactor) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].material.emissiveFactor = " << formatVector3(mesh.submeshes[s].material.emissiveFactor) << ";\n";

        code << ind << "mesh.submeshes[" << idx << "].primitiveType = " << formatPrimitiveType(mesh.submeshes[s].primitiveType) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].faceCulling = " << formatBool(mesh.submeshes[s].faceCulling) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].textureShadow = " << formatBool(mesh.submeshes[s].textureShadow) << ";\n";
        code << ind << "mesh.submeshes[" << idx << "].textureRect = " << formatRect(mesh.submeshes[s].textureRect) << ";\n";
    }
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<MeshComponent>(" << entity << ", mesh);\n";
        else
            code << ind << sceneName << ".addComponent<MeshComponent>(" << entity << ", mesh);\n";
    }
    return code.str();
}

std::string Editor::Factory::createUIComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<UIComponent>(entity)) return "";
    UIComponent& ui = scene->getComponent<UIComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "UIComponent ui;\n";
    code << ind << "ui.color = " << formatVector4(ui.color) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<UIComponent>(" << entity << ", ui);\n";
        else
            code << ind << sceneName << ".addComponent<UIComponent>(" << entity << ", ui);\n";
    }
    return code.str();
}

std::string Editor::Factory::createUILayoutComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<UILayoutComponent>(entity)) return "";
    UILayoutComponent& layout = scene->getComponent<UILayoutComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "UILayoutComponent layout;\n";
    code << ind << "layout.width = " << formatUInt(layout.width) << ";\n";
    code << ind << "layout.height = " << formatUInt(layout.height) << ";\n";
    code << ind << "layout.ignoreScissor = " << formatBool(layout.ignoreScissor) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<UILayoutComponent>(" << entity << ", layout);\n";
        else
            code << ind << sceneName << ".addComponent<UILayoutComponent>(" << entity << ", layout);\n";
    }
    return code.str();
}

std::string Editor::Factory::createImageComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<ImageComponent>(entity)) return "";
    ImageComponent& image = scene->getComponent<ImageComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "ImageComponent image;\n";
    code << ind << "image.patchMarginLeft = " << formatUInt(image.patchMarginLeft) << ";\n";
    code << ind << "image.patchMarginRight = " << formatUInt(image.patchMarginRight) << ";\n";
    code << ind << "image.patchMarginTop = " << formatUInt(image.patchMarginTop) << ";\n";
    code << ind << "image.patchMarginBottom = " << formatUInt(image.patchMarginBottom) << ";\n";
    code << ind << "image.textureScaleFactor = " << formatFloat(image.textureScaleFactor) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<ImageComponent>(" << entity << ", image);\n";
        else
            code << ind << sceneName << ".addComponent<ImageComponent>(" << entity << ", image);\n";
    }
    return code.str();
}

std::string Editor::Factory::createSpriteComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<SpriteComponent>(entity)) return "";
    SpriteComponent& sprite = scene->getComponent<SpriteComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "SpriteComponent sprite;\n";
    code << ind << "sprite.width = " << formatUInt(sprite.width) << ";\n";
    code << ind << "sprite.height = " << formatUInt(sprite.height) << ";\n";
    code << ind << "sprite.pivotPreset = " << formatPivotPreset(sprite.pivotPreset) << ";\n";
    code << ind << "sprite.textureScaleFactor = " << formatFloat(sprite.textureScaleFactor) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<SpriteComponent>(" << entity << ", sprite);\n";
        else
            code << ind << sceneName << ".addComponent<SpriteComponent>(" << entity << ", sprite);\n";
    }
    return code.str();
}

std::string Editor::Factory::createLightComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<LightComponent>(entity)) return "";
    LightComponent& light = scene->getComponent<LightComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "LightComponent light;\n";
    code << ind << "light.type = " << formatLightType(light.type) << ";\n";
    code << ind << "light.direction = " << formatVector3(light.direction) << ";\n";
    code << ind << "light.shadows = " << formatBool(light.shadows) << ";\n";
    code << ind << "light.intensity = " << formatFloat(light.intensity) << ";\n";
    code << ind << "light.range = " << formatFloat(light.range) << ";\n";
    code << ind << "light.color = " << formatVector3(light.color) << ";\n";
    code << ind << "light.innerConeCos = " << formatFloat(light.innerConeCos) << ";\n";
    code << ind << "light.outerConeCos = " << formatFloat(light.outerConeCos) << ";\n";
    code << ind << "light.shadowBias = " << formatFloat(light.shadowBias) << ";\n";
    code << ind << "light.mapResolution = " << formatUInt(light.mapResolution) << ";\n";
    code << ind << "light.automaticShadowCamera = " << formatBool(light.automaticShadowCamera) << ";\n";
    code << ind << "light.shadowCameraNearFar.x = " << formatFloat(light.shadowCameraNearFar.x) << ";\n";
    code << ind << "light.shadowCameraNearFar.y = " << formatFloat(light.shadowCameraNearFar.y) << ";\n";
    code << ind << "light.numShadowCascades = " << formatUInt(light.numShadowCascades) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<LightComponent>(" << entity << ", light);\n";
        else
            code << ind << sceneName << ".addComponent<LightComponent>(" << entity << ", light);\n";
    }
    return code.str();
}

std::string Editor::Factory::createCameraComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<CameraComponent>(entity)) return "";
    CameraComponent& camera = scene->getComponent<CameraComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "CameraComponent camera;\n";
    code << ind << "camera.type = " << formatInt((int)camera.type) << ";\n";
    code << ind << "camera.target = " << formatVector3(camera.target) << ";\n";
    code << ind << "camera.up = " << formatVector3(camera.up) << ";\n";
    code << ind << "camera.leftClip = " << formatFloat(camera.leftClip) << ";\n";
    code << ind << "camera.rightClip = " << formatFloat(camera.rightClip) << ";\n";
    code << ind << "camera.bottomClip = " << formatFloat(camera.bottomClip) << ";\n";
    code << ind << "camera.topClip = " << formatFloat(camera.topClip) << ";\n";
    code << ind << "camera.yfov = " << formatFloat(camera.yfov) << ";\n";
    code << ind << "camera.aspect = " << formatFloat(camera.aspect) << ";\n";
    code << ind << "camera.nearClip = " << formatFloat(camera.nearClip) << ";\n";
    code << ind << "camera.farClip = " << formatFloat(camera.farClip) << ";\n";
    code << ind << "camera.renderToTexture = " << formatBool(camera.renderToTexture) << ";\n";
    code << ind << "camera.transparentSort = " << formatBool(camera.transparentSort) << ";\n";
    code << ind << "camera.useTarget = " << formatBool(camera.useTarget) << ";\n";
    code << ind << "camera.autoResize = " << formatBool(camera.autoResize) << ";\n";
    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<CameraComponent>(" << entity << ", camera);\n";
        else
            code << ind << sceneName << ".addComponent<CameraComponent>(" << entity << ", camera);\n";
    }
    return code.str();
}

std::string Editor::Factory::createScriptComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    if (!scene->findComponent<ScriptComponent>(entity)) return "";
    ScriptComponent& script = scene->getComponent<ScriptComponent>(entity);
    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "ScriptComponent script;\n";

    for (size_t i = 0; i < script.scripts.size(); i++) {
        std::string idx = std::to_string(i);
        code << ind << "script.scripts.push_back(ScriptEntry());\n";
        code << ind << "script.scripts[" << idx << "].type = " << formatScriptType(script.scripts[i].type) << ";\n";
        code << ind << "script.scripts[" << idx << "].path = " << formatString(script.scripts[i].path) << ";\n";
        code << ind << "script.scripts[" << idx << "].headerPath = " << formatString(script.scripts[i].headerPath) << ";\n";
        code << ind << "script.scripts[" << idx << "].className = " << formatString(script.scripts[i].className) << ";\n";
        code << ind << "script.scripts[" << idx << "].enabled = " << formatBool(script.scripts[i].enabled) << ";\n";
    }

    if (!sceneName.empty()) {
        if (sceneName == "scene")
            code << ind << sceneName << "->addComponent<ScriptComponent>(" << entity << ", script);\n";
        else
            code << ind << sceneName << ".addComponent<ScriptComponent>(" << entity << ", script);\n";
    }
    return code.str();
}

std::string Editor::Factory::createComponent(int indentSpaces, Scene* scene, Entity entity, ComponentType componentType, std::string sceneName) {
    switch (componentType) {
        case ComponentType::Transform: return createTransform(indentSpaces, scene, entity, sceneName);
        case ComponentType::MeshComponent: return createMeshComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::UIComponent: return createUIComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::UILayoutComponent: return createUILayoutComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::ImageComponent: return createImageComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::SpriteComponent: return createSpriteComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::LightComponent: return createLightComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::CameraComponent: return createCameraComponent(indentSpaces, scene, entity, sceneName);
        case ComponentType::ScriptComponent: return createScriptComponent(indentSpaces, scene, entity, sceneName);
        default: return "";
    }
}

std::string Editor::Factory::createAllComponents(int indentSpaces, Scene* scene, Entity entity, std::string sceneName) {
    // Find all components for this entity
    std::vector<ComponentType> components = Catalog::findComponents(scene, entity);

    std::ostringstream code;
    const std::string ind = indentation(indentSpaces);
    code << ind << "// Entity components initialization\n";

    bool first = true;
    for (ComponentType componentType : components) {
        std::string componentCode = createComponent(indentSpaces, scene, entity, componentType, sceneName);
        if (!componentCode.empty()) {
            if (!first) {
                code << "\n";
            }
            code << componentCode;
            first = false;
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

    out << ind << "void init_" << mainSceneVar << "(Scene* scene){\n";

    const std::string ind2 = indentation(indentSpaces+4);

    for (Entity entity : entities) {
        out << ind2 << "// Entity " << entity << "\n";
        out << ind2 << "scene->recreateEntity(" << entity << ");\n\n";

        // Create and set all components
        std::string componentsCode = createAllComponents(indentSpaces+4, scene, entity, "scene");
        out << componentsCode;
    }

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

    bool first = true;
    for (ComponentType componentType : components) {
        std::string componentCode = setComponent(scene, entity, componentType);
        if (!componentCode.empty()) {
            if (!first) {
                code << "\n";
            }
            code << componentCode;
            first = false;
        }
    }

    return code.str();
}