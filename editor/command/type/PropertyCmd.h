#pragma once

#include "command/Command.h"
#include "Scene.h"
#include "math/Vector3.h"
#include "ecs/Entity.h"
#include "component/Transform.h"
#include "Catalog.h"


namespace Supernova::Editor{

    template<typename T>
    struct PropertyCmdValue{
        T oldValue;
        T newValue;
    };

    template<typename T>
    class PropertyCmd: public Command{

    private:
        SceneProject* sceneProject;
        ComponentType type;
        std::string propertyName;
        int updateFlags;
        bool wasModified;

        std::map<Entity,PropertyCmdValue<T>> values;

    public:

        PropertyCmd(SceneProject* sceneProject, Entity entity, ComponentType type, std::string propertyName, int updateFlags, T newValue){
            this->sceneProject = sceneProject;
            this->type = type;
            this->propertyName = propertyName;
            this->updateFlags = updateFlags;

            this->values[entity].newValue = newValue;
            this->wasModified = sceneProject->isModified;
        }

        bool execute(){
            for (auto& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(sceneProject->scene, entity, type, propertyName);

                value.oldValue = T(*valueRef);
                *valueRef = value.newValue;

                Catalog::updateEntity(sceneProject->scene, entity, updateFlags);
            }

            sceneProject->isModified = true;

            return true;
        }

        void undo(){
            for (auto const& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(sceneProject->scene, entity, type, propertyName);

                *valueRef = value.oldValue;

                Catalog::updateEntity(sceneProject->scene, entity, updateFlags);
            }

            sceneProject->isModified = wasModified;
        }

        bool mergeWith(Editor::Command* otherCommand){
            PropertyCmd* otherCmd = dynamic_cast<PropertyCmd*>(otherCommand);
            if (otherCmd != nullptr){
                if (sceneProject->scene == otherCmd->sceneProject->scene && propertyName == otherCmd->propertyName){
                    for (auto const& [otherEntity, otherValue] : otherCmd->values){
                        if (values.find(otherEntity) != values.end()) {
                            values[otherEntity].oldValue = otherValue.oldValue;
                        }else{
                            values[otherEntity] = otherValue;
                        }
                    }
                    updateFlags |= otherCmd->updateFlags;
                    wasModified = wasModified && otherCmd->wasModified;
                    return true;
                }
            }

            return false;
        }

        void finalize(){
            Command::finalize();

            for (auto const& [entity, value] : values){
                Event e;
                e.type = EventType::ComponentChanged;
                e.sceneId = sceneProject->id;
                e.entity = entity;
                e.compType = ComponentType::Transform;
                Project::getEventBus().publish(e);
            }
        }
    };

}