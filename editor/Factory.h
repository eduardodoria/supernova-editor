#pragma once

#include "Scene.h"
#include "Catalog.h"
#include <string>
#include <sstream>
#include "Project.h"

namespace Supernova::Editor{

    class Factory{
    private:
        static std::string indentation(int spaces);
        static std::string formatVector2(const Vector2& v);
        static std::string formatVector3(const Vector3& v);
        static std::string formatVector4(const Vector4& v);
        static std::string formatRect(const Rect& r);
        static std::string formatPrimitiveType(PrimitiveType type);
        static std::string formatPivotPreset(PivotPreset preset);
        static std::string formatLightType(LightType type);
        static std::string formatCameraType(CameraType type);
        static std::string formatScriptType(ScriptType type);
        static std::string formatQuaternion(const Quaternion& q);
        static std::string formatBool(bool value);
        static std::string formatFloat(float value);
        static std::string formatInt(int value);
        static std::string formatUInt(unsigned int value);
        static std::string formatString(const std::string& value);
        static std::string formatAttributeType(AttributeType type);
        static std::string formatAttributeDataType(AttributeDataType type);
        static std::string formatTextureFilter(TextureFilter filter);
        static std::string formatTextureWrap(TextureWrap wrap);

        static std::string formatTexture(int indentSpaces, const Texture& texture, const std::string& variableName, const std::string& projectPath);

        static std::string formatPropertyValue(const PropertyData& property, const std::string& propertyName);

        static void addComponentCode(std::ostringstream& code, const std::string& ind, const std::string& sceneName, const std::string& entityName, Entity entity, const std::string& componentType, const std::string& varName);

    public:
        Factory();

        static std::string toIdentifier(const std::string& name);

        static std::string createComponent(int indentSpaces, Scene* scene, Entity entity, ComponentType componentType, const std::string& projectPath, std::string sceneName = "", std::string entityName = "");
        static std::string createAllComponents(int indentSpaces, Scene* scene, Entity entity, const std::string& projectPath, std::string sceneName = "", std::string entityName = "");
        static std::string createScene(int indentSpaces, Scene* scene, std::string name, std::vector<Entity> entities, Entity camera, const std::string& projectPath);

        static std::string setComponent(Scene* scene, Entity entity, ComponentType componentType, const std::string& projectPath);
        static std::string setAllComponents(Scene* scene, Entity entity, const std::string& projectPath);

        static std::string createTransform(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createMeshComponent(int indentSpaces, Scene* scene, Entity entity, const std::string& projectPath, std::string sceneName = "", std::string entityName = "");
        static std::string createUIComponent(int indentSpaces, Scene* scene, Entity entity, const std::string& projectPath, std::string sceneName = "", std::string entityName = "");
        static std::string createUILayoutComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createImageComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createSpriteComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createLightComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createCameraComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
        static std::string createScriptComponent(int indentSpaces, Scene* scene, Entity entity, std::string sceneName = "", std::string entityName = "");
    };

}