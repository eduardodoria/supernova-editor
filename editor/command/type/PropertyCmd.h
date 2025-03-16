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

        void updateEntity(Entity entity){
            if (updateFlags & UpdateFlags_Transform){
                scene->getComponent<Transform>(entity).needUpdate = true;
            }
            if (updateFlags & UpdateFlags_Mesh_Reload){
                scene->getComponent<MeshComponent>(entity).needReload = true;
            }
            if (updateFlags & UpdateFlags_Mesh_Texture){
                unsigned int numSubmeshes = scene->getComponent<MeshComponent>(entity).numSubmeshes;
                for (unsigned int i = 0; i < numSubmeshes; i++){
                    scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
                }
            }
            if (updateFlags & UpdateFlags_UI_Reload){
                scene->getComponent<UIComponent>(entity).needReload = true;
            }
            if (updateFlags & UpdateFlags_UI_Texture){
                scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
            }
        }

    public:

        PropertyCmd(Scene* scene, Entity entity, ComponentType type, std::string propertyName, int updateFlags, T newValue){
            this->scene = scene;
            this->type = type;
            this->propertyName = propertyName;
            this->updateFlags = updateFlags;

            this->values[entity].newValue = newValue;
        }

        void execute(){
            for (auto& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

                value.oldValue = T(*valueRef);
                *valueRef = value.newValue;

                updateEntity(entity);
            }
        }

        void undo(){
            for (auto const& [entity, value] : values){
                T* valueRef = Catalog::getPropertyRef<T>(scene, entity, type, propertyName);

                *valueRef = value.oldValue;

                updateEntity(entity);
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

#endif /* CHANGEPROPERTYCMD_H */