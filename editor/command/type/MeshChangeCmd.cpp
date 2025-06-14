#include "MeshChangeCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::MeshChangeCmd::MeshChangeCmd(SceneProject* sceneProject, Entity entity, MeshComponent mesh){
    this->sceneProject = sceneProject;
    this->entity = entity;
    this->newMesh = Stream::encodeMeshComponent(mesh);

    this->wasModified = sceneProject->isModified;
}

bool Editor::MeshChangeCmd::execute(){
    oldMesh = Stream::encodeMeshComponent(sceneProject->scene->getComponent<MeshComponent>(entity));
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(newMesh);

    sceneProject->isModified = true;

    return true;
}

void Editor::MeshChangeCmd::undo(){
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);

    sceneProject->isModified = wasModified;
}

bool Editor::MeshChangeCmd::mergeWith(Editor::Command* otherCommand){
    MeshChangeCmd* otherCmd = dynamic_cast<MeshChangeCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneProject == otherCmd->sceneProject && entity == otherCmd->entity){
            this->oldMesh = otherCmd->oldMesh;

            this->wasModified = this->wasModified && otherCmd->wasModified;
            return true;
        }
    }
    return false;
}