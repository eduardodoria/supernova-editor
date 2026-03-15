#include "RemoveEntityFromBundleCmd.h"

using namespace Supernova;

Editor::RemoveEntityFromBundleCmd::RemoveEntityFromBundleCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->parent = parent;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::RemoveEntityFromBundleCmd::execute(){
    recovery = project->removeEntityFromBundle(sceneId, entity, false);

    bool result = !recovery.empty();
    if (result){
        project->getScene(sceneId)->isModified = true;
    }

    return result;
}

void Editor::RemoveEntityFromBundleCmd::undo(){
    project->addEntityToBundle(sceneId, recovery, parent, false);

    project->getScene(sceneId)->isModified = wasModified;
}

bool Editor::RemoveEntityFromBundleCmd::mergeWith(Editor::Command* otherCommand){

    return false;
}
