#include "AddEntityToBundleCmd.h"

using namespace Supernova;

Editor::AddEntityToBundleCmd::AddEntityToBundleCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->parent = parent;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::AddEntityToBundleCmd::execute(){
    bool result = project->addEntityToBundle(sceneId, entity, parent, false);

    if (result){
        project->getScene(sceneId)->isModified = true;
    }

    return result;
}

void Editor::AddEntityToBundleCmd::undo(){
    project->removeEntityFromBundle(sceneId, entity, false);

    project->getScene(sceneId)->isModified = wasModified;
}

bool Editor::AddEntityToBundleCmd::mergeWith(Editor::Command* otherCommand){

    return false;
}
