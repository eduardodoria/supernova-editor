#include "ComponentToLocalCmd.h"

using namespace Supernova;

Editor::ComponentToLocalCmd::ComponentToLocalCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->componentType = componentType;
}

bool Editor::ComponentToLocalCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        //Project::addEntityComponent(scene, entity, componentType, sceneProject->entities);

        //if (project->isEntityShared(sceneId, entity)){
        //    project->addComponentToSharedGroup(sceneId, entity, componentType, false);
        //}

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::ComponentToLocalCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        //Project::removeEntityComponent(scene, entity, componentType, sceneProject->entities);

        //if (project->isEntityShared(sceneId, entity)){
        //    project->removeComponentToSharedGroup(sceneId, entity, componentType, false, false);
        //}

        sceneProject->isModified = true;
    }
}

bool Editor::ComponentToLocalCmd::mergeWith(Command* otherCommand){

    return false;
}