#ifndef CHANGEPROPERTYCMD_H
#define CHANGEPROPERTYCMD_H

#include "command/Command.h"
#include "Scene.h"
#include "math/Vector3.h"
#include "ecs/Entity.h"
#include "component/Transform.h"
#include "Structure.h"


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
            T* valueRef = Structure::getPropertyRef<T>(scene, entity, type, propertyName);

            oldValue = T(*valueRef);
            *valueRef = newValue;

            scene->getComponent<Transform>(entity).needUpdate = true;
        }

        void undo(){
            T* valueRef = Structure::getPropertyRef<T>(scene, entity, type, propertyName);

            *valueRef = oldValue;

            scene->getComponent<Transform>(entity).needUpdate = true;
        }

        bool mergeWith(Editor::Command* olderCommand){
            T* valueRef = Structure::getPropertyRef<T>(scene, entity, type, propertyName);
            ChangePropertyCmd* olderCmd = dynamic_cast<ChangePropertyCmd*>(olderCommand);
            if (olderCmd != nullptr){
                T* olderValueRef = Structure::getPropertyRef<T>(olderCmd->scene, olderCmd->entity, olderCmd->type, olderCmd->propertyName);
                if (valueRef == olderValueRef){
                    this->oldValue = olderCmd->oldValue;
                    return true;
                }
            }

            return false;
        }
    };

}

#endif /* CHANGEPROPERTYCMD_H */