#include "ChangeEntityNameCmd.h"

using namespace Supernova;

Editor::ChangeEntityNameCmd::ChangeEntityNameCmd(Scene* scene, Entity entity, std::string name){
    this->scene = scene;
    this->entity = entity;
    this->newName = name;
}

void Editor::ChangeEntityNameCmd::execute(){
    oldName = scene->getEntityName(entity);
    scene->setEntityName(entity, newName);
}

void Editor::ChangeEntityNameCmd::undo(){
    scene->setEntityName(entity, oldName);
}

bool Editor::ChangeEntityNameCmd::mergeWith(Editor::Command* otherCommand){
    ChangeEntityNameCmd* otherCmd = dynamic_cast<ChangeEntityNameCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (scene == otherCmd->scene && entity == otherCmd->entity){
            this->oldName = otherCmd->oldName;
            return true;
        }
    }
    return false;
}