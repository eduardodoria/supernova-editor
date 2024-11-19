#ifndef CHANGEPROPERTYCMD_H
#define CHANGEPROPERTYCMD_H

#include "command/Command.h"
#include "Scene.h"
#include "math/Vector3.h"
#include "ecs/Entity.h"
#include "component/Transform.h"
#include "Metadata.h"


namespace Supernova::Editor{

    template<typename T>
    class ChangePropertyCmd: public Command{

    private:
        T oldValue;
        T newValue;

        Scene* scene;
        Entity entity;
        ComponentType type;
        std::string propertyName;

    public:

        ChangePropertyCmd(Scene* scene, Entity entity, ComponentType type, std::string propertyName, T newValue){
            this->scene = scene;
            this->entity = entity;
            this->type = type;
            this->propertyName = propertyName;
            this->newValue = newValue;
        }

        void execute(){
            T* valueRef = Metadata::getPropertyRef<T>(scene, entity, type, propertyName);

            oldValue = T(*valueRef);
            *valueRef = newValue;

            scene->getComponent<Transform>(entity).needUpdate = true;
        }

        void undo(){
            T* valueRef = Metadata::getPropertyRef<T>(scene, entity, type, propertyName);

            *valueRef = oldValue;

            scene->getComponent<Transform>(entity).needUpdate = true;
        }

        bool mergeWith(Editor::Command* otherCommand){
            T* valueRef = Metadata::getPropertyRef<T>(scene, entity, type, propertyName);
            ChangePropertyCmd* otherCmd = dynamic_cast<ChangePropertyCmd*>(otherCommand);
            if (otherCmd != nullptr){
                T* olderValueRef = Metadata::getPropertyRef<T>(otherCmd->scene, otherCmd->entity, otherCmd->type, otherCmd->propertyName);
                if (valueRef == olderValueRef){
                    this->oldValue = otherCmd->oldValue;
                    return true;
                }
            }

            return false;
        }
    };

}

#endif /* CHANGEPROPERTYCMD_H */