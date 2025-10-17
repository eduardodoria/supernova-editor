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
        Project* project;
        uint32_t sceneId;
        ComponentType type;
        std::string propertyName;
        int updateFlags;
        bool wasModified;

        std::map<Entity,PropertyCmdValue<T>> values;

    public:

        PropertyCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType type, std::string propertyName, T newValue){
            this->project = project;
            this->sceneId = sceneId;
            this->type = type;
            this->propertyName = propertyName;
            this->updateFlags = updateFlags;

            this->values[entity].newValue = newValue;
            this->wasModified = project->getScene(sceneId)->isModified;
        }

        bool execute() override{
            SceneProject* sceneProject = project->getScene(sceneId);
            if (!sceneProject){
                return false;
            }
            for (auto& [entity, value] : values){
                PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, type, propertyName);
                T* valueRef = static_cast<T*>(prop.ref);

                value.oldValue = T(*valueRef);
                *valueRef = value.newValue;

                Catalog::updateEntity(sceneProject->scene, entity, prop.updateFlags);

                if (project->isEntityShared(sceneId, entity)){
                    project->sharedGroupPropertyChanged(sceneId, entity, type, {propertyName});
                }
            }

            sceneProject->isModified = true;

            return true;
        }

        void undo() override{
            SceneProject* sceneProject = project->getScene(sceneId);
            if (!sceneProject){
                return;
            }
            for (auto const& [entity, value] : values){
                PropertyData prop = Catalog::getProperty(sceneProject->scene, entity, type, propertyName);
                T* valueRef = static_cast<T*>(prop.ref);

                *valueRef = value.oldValue;

                Catalog::updateEntity(sceneProject->scene, entity, prop.updateFlags);

                if (project->isEntityShared(sceneId, entity)){
                    project->sharedGroupPropertyChanged(sceneId, entity, type, {propertyName});
                }
            }

            sceneProject->isModified = wasModified;
        }

        bool mergeWith(Editor::Command* otherCommand) override{
            PropertyCmd* otherCmd = dynamic_cast<PropertyCmd*>(otherCommand);
            if (otherCmd != nullptr){
                if (sceneId == otherCmd->sceneId && propertyName == otherCmd->propertyName){
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

    };

}