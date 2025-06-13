#include "MeshGeometryCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::MeshGeometryCmd::MeshGeometryCmd(SceneProject* sceneProject, Entity entity, MeshComponent mesh){
    this->sceneProject = sceneProject;
    this->entity = entity;
    this->newMesh = Stream::encodeMeshComponent(mesh);

    this->wasModified = sceneProject->isModified;
}

bool Editor::MeshGeometryCmd::execute(){
    oldMesh = Stream::encodeMeshComponent(sceneProject->scene->getComponent<MeshComponent>(entity));
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(newMesh);

    sceneProject->isModified = true;

    return true;
}

void Editor::MeshGeometryCmd::undo(){
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);

    sceneProject->isModified = wasModified;
}

bool Editor::MeshGeometryCmd::mergeWith(Editor::Command* otherCommand){
    MeshGeometryCmd* otherCmd = dynamic_cast<MeshGeometryCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneProject == otherCmd->sceneProject && entity == otherCmd->entity){
            this->oldMesh = otherCmd->oldMesh;

            this->wasModified = this->wasModified && otherCmd->wasModified;
            return true;
        }
    }
    return false;
}