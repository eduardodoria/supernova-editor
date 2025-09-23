#include "MeshChangeCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::MeshChangeCmd::MeshChangeCmd(Project* project, uint32_t sceneId, Entity entity, MeshComponent mesh){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->newMesh = Stream::encodeMeshComponent(mesh);

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::MeshChangeCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    oldMesh = Stream::encodeMeshComponent(sceneProject->scene->getComponent<MeshComponent>(entity));
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(newMesh);

    sceneProject->isModified = true;

    return true;
}

void Editor::MeshChangeCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);

    sceneProject->isModified = wasModified;
}

bool Editor::MeshChangeCmd::mergeWith(Editor::Command* otherCommand){
    MeshChangeCmd* otherCmd = dynamic_cast<MeshChangeCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId && entity == otherCmd->entity){
            this->oldMesh = otherCmd->oldMesh;

            this->wasModified = this->wasModified && otherCmd->wasModified;
            return true;
        }
    }
    return false;
}

void Editor::MeshChangeCmd::commit(){
    if (project->isEntityShared(sceneId, entity)){
        project->sharedGroupPropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
    }
}