#ifndef CHANGEPROPERTYCMD_H
#define CHANGEPROPERTYCMD_H

#include "command/Command.h"
#include "Scene.h"
#include "math/Vector3.h"
#include "ecs/Entity.h"
#include "component/Transform.h"
#include "Catalog.h"


namespace Supernova::Editor{

    template<typename T>
    class PropertyCmd: public Command{

    private:
        T oldValue;
        T newValue;

        Scene* scene;
        Entity entity;
        ComponentType type;
        std::string propertyName;
        int updateFlags;

        void updateEntity(){
            if (updateFlags & UpdateFlags_Transform){
                scene->getComponent<Transform>(entity).needUpdate = true;
            }
            if (updateFlags & UpdateFlags_MeshReload){
                scene->getComponent<MeshComponent>(entity).needReload = true;
            }
        }

    public:

        PropertyCmd(Scene* scene, Entity entity, ComponentType type, std::string propertyName, int updateFlags, T newValue){
            this->scene = scene;
            this->entity = entity;
            this->type = type;
            this->propertyName = propertyName;
            this->updateFlags = updateFlags;
            this->newValue = newValue;
        }

        void execute(){
            T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

            oldValue = T(*valueRef);
            *valueRef = newValue;

            updateEntity();
        }

        void undo(){
            T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

            *valueRef = oldValue;

            updateEntity();
        }

        bool mergeWith(Editor::Command* otherCommand){
            T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);
            PropertyCmd* otherCmd = dynamic_cast<PropertyCmd*>(otherCommand);
            if (otherCmd != nullptr){
                T* olderValueRef = Catalog::getPropertyRef<T>(otherCmd->scene, otherCmd->entity, otherCmd->type, otherCmd->propertyName);
                if (valueRef == olderValueRef){
                    this->oldValue = otherCmd->oldValue;
                    this->updateFlags |= otherCmd->updateFlags;
                    return true;
                }
            }

            return false;
        }
    };

}

#endif /* CHANGEPROPERTYCMD_H */