#include "ObjectTransformCmd.h"

using namespace Supernova;

Editor::ObjectTransformCmd::ObjectTransformCmd(SceneProject* sceneProject, Entity entity, Matrix4 localMatrix){
    this->sceneProject = sceneProject;
    
    localMatrix.decomposeStandard(props[entity].newPosition, props[entity].newScale, props[entity].newRotation);
}

Editor::ObjectTransformCmd::ObjectTransformCmd(SceneProject* sceneProject, Entity entity, Vector3 position, Quaternion rotation, Vector3 scale){
    this->sceneProject = sceneProject;

    props[entity].newPosition = position;
    props[entity].newRotation = rotation;
    props[entity].newScale = scale;
}

bool Editor::ObjectTransformCmd::execute(){
    for (auto& [entity, property] : props){
        if (Transform* transform = sceneProject->scene->findComponent<Transform>(entity)){
            property.oldPosition = transform->position;
            property.oldRotation = transform->rotation;
            property.oldScale = transform->scale;

            transform->position = property.newPosition;
            transform->rotation = property.newRotation;
            transform->scale = property.newScale;

            transform->needUpdate = true;
        }
    }

    sceneProject->isModified = true;

    return true;
}

void Editor::ObjectTransformCmd::undo(){
    for (auto const& [entity, property] : props){
        if (Transform* transform = sceneProject->scene->findComponent<Transform>(entity)){
            transform->position = property.oldPosition;
            transform->rotation = property.oldRotation;
            transform->scale = property.oldScale;

            transform->needUpdate = true;
        }
    }

    sceneProject->isModified = true;
}

bool Editor::ObjectTransformCmd::mergeWith(Editor::Command* otherCommand){
    ObjectTransformCmd* otherCmd = dynamic_cast<ObjectTransformCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneProject == otherCmd->sceneProject){

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