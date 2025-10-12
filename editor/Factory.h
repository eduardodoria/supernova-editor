#pragma once

#include "Scene.h"
#include "Catalog.h"
#include <string>
#include <sstream>

namespace Supernova::Editor{

    class Factory{
    private:
        static std::string formatVector2(const Vector2& v);
        static std::string formatVector3(const Vector3& v);
        static std::string formatVector4(const Vector4& v);
        static std::string formatQuaternion(const Quaternion& q);
        static std::string formatBool(bool value);
        static std::string formatFloat(float value);
        static std::string formatInt(int value);
        static std::string formatUInt(unsigned int value);
        static std::string formatString(const std::string& value);

        static std::string formatPropertyValue(const PropertyData& property, const std::string& propertyName);

    public:
        Factory();

        static std::string createComponent(Scene* scene, Entity entity, ComponentType componentType);
        static std::string createAllComponents(Scene* scene, Entity entity);
    };

}