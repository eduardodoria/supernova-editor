#include "EntityNameCmd.h"

using namespace Supernova;

Editor::EntityNameCmd::EntityNameCmd(SceneProject* sceneProject, Entity entity, std::string name){
    this->sceneProject = sceneProject;
    this->entity = entity;
    this->newName = name;
}

void Editor::EntityNameCmd::execute(){
    oldName = sceneProject->scene->getEntityName(entity);
    sceneProject->scene->setEntityName(entity, newName);

    sceneProject->isModified = true;
}

void Editor::EntityNameCmd::undo(){
    sceneProject->scene->setEntityName(entity, oldName);

    sceneProject->isModified = true;
}

bool Editor::EntityNameCmd::mergeWith(Editor::Command* otherCommand){
    EntityNameCmd* otherCmd = dynamic_cast<EntityNameCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneProject == otherCmd->sceneProject && entity == otherCmd->entity){
            this->oldName = otherCmd->oldName;
            return true;
        }
    }
    return false;
}