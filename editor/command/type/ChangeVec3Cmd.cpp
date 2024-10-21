
#include "ChangeVec3Cmd.h"

using namespace Supernova;

//Editor::ChangeVec3Cmd::ChangeVec3Cmd(Scene* scene, Entity entity, ComponentType type, std::string property, Vector3 newVector){
//    std::vector<PropertyData> properties = Structure::getProperties();
//}

Editor::ChangeVec3Cmd::ChangeVec3Cmd(Vector3& originalVector, Vector3 newVector): vector(originalVector){
    this->oldVector = Vector3();
    this->newVector = newVector;
    this->transform = nullptr;
}

Editor::ChangeVec3Cmd::ChangeVec3Cmd(Vector3& originalVector, Vector3 newVector, Transform* transform): vector(originalVector){
    this->oldVector = Vector3();
    this->newVector = newVector;
    this->transform = transform;
}

void Editor::ChangeVec3Cmd::execute(){
    oldVector = Vector3(vector);
    vector = newVector;

    if (transform){
        transform->needUpdate = true;
    }
}

void Editor::ChangeVec3Cmd::undo(){
    vector = oldVector;

    if (transform){
        transform->needUpdate = true;
    }
}

bool Editor::ChangeVec3Cmd::mergeWith(Editor::Command* olderCommand){
        ChangeVec3Cmd* changeVec3Cmd = dynamic_cast<ChangeVec3Cmd*>(olderCommand);
        if (changeVec3Cmd != nullptr){
            if (&this->vector == &changeVec3Cmd->vector){
                this->oldVector = changeVec3Cmd->oldVector;
                return true;
            }
        }

    return false;
}