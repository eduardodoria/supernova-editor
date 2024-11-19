#include "ChangeObjTransfCmd.h"

using namespace Supernova;

Editor::ChangeObjTransfCmd::ChangeObjTransfCmd(Scene* scene, Entity entity, Matrix4 localMatrix){
    this->scene = scene;
    
    localMatrix.decomposeStandard(props[entity].newPosition, props[entity].newScale, props[entity].newRotation);
}

Editor::ChangeObjTransfCmd::ChangeObjTransfCmd(Scene* scene, Entity entity, Vector3 position, Quaternion rotation, Vector3 scale){
    this->scene = scene;

    props[entity].newPosition = position;
    props[entity].newRotation = rotation;
    props[entity].newScale = scale;
}

void Editor::ChangeObjTransfCmd::execute(){
    for (auto& [entity, property] : props){
        if (Transform* transform = scene->findComponent<Transform>(entity)){
            property.oldPosition = transform->position;
            property.oldRotation = transform->rotation;
            property.oldScale = transform->scale;

            transform->position = property.newPosition;
            transform->rotation = property.newRotation;
            transform->scale = property.newScale;

            transform->needUpdate = true;
        }
    }
}

void Editor::ChangeObjTransfCmd::undo(){
    for (auto const& [entity, property] : props){
        if (Transform* transform = scene->findComponent<Transform>(entity)){
            transform->position = property.oldPosition;
            transform->rotation = property.oldRotation;
            transform->scale = property.oldScale;

            transform->needUpdate = true;
        }
    }
}

bool Editor::ChangeObjTransfCmd::mergeWith(Editor::Command* otherCommand){
    ChangeObjTransfCmd* otherCmd = dynamic_cast<ChangeObjTransfCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (scene == otherCmd->scene){

            for (auto const& [otherEntity, otherProperty] : otherCmd->props){
                if (props.find(otherEntity) != props.end()) {
                    props[otherEntity].oldPosition = otherProperty.oldPosition;
                    props[otherEntity].oldRotation = otherProperty.oldRotation;
                    props[otherEntity].oldScale = otherProperty.oldScale;
                }else{
                    props[otherEntity] = otherProperty;
                }
            }

            return true;
        }
    }

    return false;
}