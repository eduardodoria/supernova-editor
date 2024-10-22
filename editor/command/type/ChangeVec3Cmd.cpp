
#include "ChangeVec3Cmd.h"

using namespace Supernova;

Editor::ChangeVec3Cmd::ChangeVec3Cmd(Scene* scene, Entity entity, ComponentType type, std::string propertyName, Vector3 newVector){
    this->scene = scene;
    this->entity = entity;
    this->type = type;
    this->propertyName = propertyName;
    this->newVector = newVector;
}

void Editor::ChangeVec3Cmd::execute(){
    Vector3* vector = Structure::getPropertyRef<Vector3>(scene, entity, type, propertyName);

    oldVector = Vector3(*vector);
    *vector = newVector;

    scene->getComponent<Transform>(entity).needUpdate = true;
}

void Editor::ChangeVec3Cmd::undo(){
    Vector3* vector = Structure::getPropertyRef<Vector3>(scene, entity, type, propertyName);

    *vector = oldVector;

    scene->getComponent<Transform>(entity).needUpdate = true;
}

bool Editor::ChangeVec3Cmd::mergeWith(Editor::Command* olderCommand){
    Vector3* vector = Structure::getPropertyRef<Vector3>(scene, entity, type, propertyName);
    ChangeVec3Cmd* changeVec3Cmd = dynamic_cast<ChangeVec3Cmd*>(olderCommand);
    if (changeVec3Cmd != nullptr){
        Vector3* olderVector = Structure::getPropertyRef<Vector3>(changeVec3Cmd->scene, changeVec3Cmd->entity, changeVec3Cmd->type, changeVec3Cmd->propertyName);
        if (vector == olderVector){
            this->oldVector = changeVec3Cmd->oldVector;
            return true;
        }
    }

    return false;
}