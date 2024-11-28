#include "EntityNameCmd.h"

using namespace Supernova;

Editor::EntityNameCmd::EntityNameCmd(Scene* scene, Entity entity, std::string name){
    this->scene = scene;
    this->entity = entity;
    this->newName = name;
}

void Editor::EntityNameCmd::execute(){
    oldName = scene->getEntityName(entity);
    scene->setEntityName(entity, newName);
}

void Editor::EntityNameCmd::undo(){
    scene->setEntityName(entity, oldName);
}

bool Editor::EntityNameCmd::mergeWith(Editor::Command* otherCommand){
    EntityNameCmd* otherCmd = dynamic_cast<EntityNameCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (scene == otherCmd->scene && entity == otherCmd->entity){
            this->oldName = otherCmd->oldName;
            return true;
        }
    }
    return false;
}