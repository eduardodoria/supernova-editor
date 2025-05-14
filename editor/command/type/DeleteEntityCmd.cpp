#include "DeleteEntityCmd.h"

#include "Stream.h"
#include "Out.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;

    DeleteEntityData entityData;
    entityData.entity = entity;
    this->entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::DeleteEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    lastSelected = project->getSelectedEntities(sceneId);

    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        DeleteEntityData& entityData = *it;

        entityData.data = Stream::encodeEntity(entityData.entity, sceneProject->scene);

        sceneProject->scene->destroyEntity(entityData.entity);

        auto ite = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entityData.entity);
        if (ite != sceneProject->entities.end()) {
            sceneProject->entities.erase(ite);
        }

        if (project->isSelectedEntity(sceneId, entityData.entity)){
            project->clearSelectedEntities(sceneId);
        }

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::DeleteEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    for (DeleteEntityData& entityData : entities){

        if (sceneProject->scene->isEntityCreated(entityData.entity)){
            Out::error("Entity '%u' already exists", entityData.entity);
            continue;
        }

        entityData.entity = Stream::decodeEntity(sceneProject->scene, entityData.data);

        sceneProject->entities.push_back(entityData.entity);
    }

    if (lastSelected.size() > 0){
        project->replaceSelectedEntities(sceneId, lastSelected);
    }

    sceneProject->isModified = wasModified;
}

bool Editor::DeleteEntityCmd::mergeWith(Editor::Command* otherCommand){
    DeleteEntityCmd* otherCmd = dynamic_cast<DeleteEntityCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            lastSelected = otherCmd->lastSelected;

            for (DeleteEntityData& otherEntityData :  otherCmd->entities){
                // insert at begin to keep deletion order
                entities.push_back(otherEntityData);
            }

            std::sort(entities.begin(), entities.end(), [](const DeleteEntityData& a, const DeleteEntityData& b) {
                return a.parent < b.parent || a.transformIndex < b.transformIndex;
            });

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}