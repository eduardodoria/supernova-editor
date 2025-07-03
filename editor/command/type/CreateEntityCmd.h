#ifndef CREATEENTITYCMD_H
#define CREATEENTITYCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <any>
#include <functional>

namespace Supernova::Editor{

    enum class EntityCreationType{
        EMPTY,
        OBJECT,
        BOX,
        PLANE,
        SPHERE,
        CYLINDER,
        CAPSULE,
        TORUS,
        IMAGE,
        SPRITE,
        POINT_LIGHT,
        DIRECTIONAL_LIGHT,
        SPOT_LIGHT
    };

    class CreateEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        std::string entityName;

        Entity entity;
        Entity parent;
        EntityCreationType type;
        std::vector<Entity> lastSelected;

        // Component type -> property name -> property setter function
        std::unordered_map<ComponentType, std::unordered_map<std::string, std::function<void(Entity)>>> propertySetters;
        int updateFlags;

    public:
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName);
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type);
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type, Entity parent);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);

        Entity getEntity();

        template<typename T>
        void addProperty(ComponentType componentType, const std::string& propertyName, T value) {
            Scene* scene = project->getScene(sceneId)->scene;

            auto properties = Catalog::getProperties(componentType, nullptr);
            auto it = properties.find(propertyName);
            if (it != properties.end()) {
                this->updateFlags |= it->second.updateFlags;
            }

            propertySetters[componentType][propertyName] = [value, scene, propertyName, componentType](Entity entity) {
                T* valueRef = Catalog::getPropertyRef<T>(scene, entity, componentType, propertyName);
                if (valueRef != nullptr) {
                    *valueRef = value;
                }
            };
        }
    };

}

#endif /* CREATEENTITYCMD_H */