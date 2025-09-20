#include "RemoveComponentCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::RemoveComponentCmd::RemoveComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->componentType = componentType;

    RemoveComponentData entityData;
    entityData.entity = entity;

    this->entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::RemoveComponentCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        for (RemoveComponentData& entityData : entities){

            fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
            SharedGroup* group = project->getSharedGroup(filepath);

            if (group && !group->hasComponentOverride(sceneId, entityData.entity, componentType)){

                entityData.recovery = project->removeComponentToSharedGroup(sceneId, entityData.entity, componentType, true, true);

            }else{

                entityData.oldComponent = Project::removeEntityComponent(scene, entityData.entity, componentType, sceneProject->entities, true);

                if (group){
                    entityData.hasOverride = group->hasComponentOverride(sceneId, entityData.entity, componentType);
                    if (entityData.hasOverride){
                        group->clearComponentOverride(sceneId, entityData.entity, componentType);
                    }
                }

            }
        }

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::RemoveComponentCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        for (RemoveComponentData& entityData : entities){
            if (entityData.recovery.size() > 0){

                project->addComponentToSharedGroup(sceneId, entityData.entity, componentType, entityData.recovery, true);

            }else{

                Project::addEntityComponent(scene, entityData.entity, componentType, sceneProject->entities, entityData.oldComponent);

                if (entityData.hasOverride){
                    fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
                    SharedGroup* group = project->getSharedGroup(filepath);

                    if (group){
                        group->setComponentOverride(sceneId, entityData.entity, componentType);
                    }
                }

            }
        }

        sceneProject->isModified = wasModified;
    }
}

bool Editor::RemoveComponentCmd::mergeWith(Command* otherCommand){
    RemoveComponentCmd* otherCmd = dynamic_cast<RemoveComponentCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            for (RemoveComponentData& otherEntityData :  otherCmd->entities){
                // insert at begin to keep deletion order
                entities.push_back(otherEntityData);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}