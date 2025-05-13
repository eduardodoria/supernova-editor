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
        Scene* scene;
        ComponentType type;
        std::string propertyName;
        int updateFlags;

        std::map<Entity,PropertyCmdValue<T>> values;

    public:

        PropertyCmd(Scene* scene, Entity entity, ComponentType type, std::string propertyName, int updateFlags, T newValue){
            this->scene = scene;
            this->type = type;
            this->propertyName = propertyName;
            this->updateFlags = updateFlags;

            this->values[entity].newValue = newValue;
        }

        bool execute(){
            for (auto& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

                value.oldValue = T(*valueRef);
                *valueRef = value.newValue;

                Catalog::updateEntity(scene, entity, updateFlags);
            }

            return true;
        }

        void undo(){
            for (auto const& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

                *valueRef = value.oldValue;

                Catalog::updateEntity(scene, entity, updateFlags);
            }
        }

        bool mergeWith(Editor::Command* otherCommand){
            PropertyCmd* otherCmd = dynamic_cast<PropertyCmd*>(otherCommand);
            if (otherCmd != nullptr){
                if (scene == otherCmd->scene && propertyName == otherCmd->propertyName){
                    for (auto const& [otherEntity, otherValue] : otherCmd->values){
                        if (values.find(otherEntity) != values.end()) {
                            values[otherEntity].oldValue = otherValue.oldValue;
                        }else{
                            values[otherEntity] = otherValue;
                        }
                    }
                    updateFlags |= otherCmd->updateFlags;
                    return true;
                }
            }

            return false;
        }
    };

}