#include "EntityNameCmd.h"

using namespace Supernova;

Editor::EntityNameCmd::EntityNameCmd(Project* project, uint32_t sceneId, Entity entity, std::string name){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->newName = name;
}

bool Editor::EntityNameCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    oldName = sceneProject->scene->getEntityName(entity);
    wasModified = project->getScene(sceneId)->isModified;

    if (project->isEntityShared(sceneId, entity)){
        project->sharedGroupNameChanged(sceneId, entity, newName, false);
    }
    sceneProject->scene->setEntityName(entity, newName);

    sceneProject->isModified = true;

    return true;
}

void Editor::EntityNameCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (project->isEntityShared(sceneId, entity)){
        project->sharedGroupNameChanged(sceneId, entity, oldName, false);
    }
    sceneProject->scene->setEntityName(entity, oldName);

    sceneProject->isModified = wasModified;
}

bool Editor::EntityNameCmd::mergeWith(Editor::Command* otherCommand){
    EntityNameCmd* otherCmd = dynamic_cast<EntityNameCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId && entity == otherCmd->entity){
            this->oldName = otherCmd->oldName;

            this->wasModified = this->wasModified && otherCmd->wasModified;
            return true;
        }
    }
    return false;
}