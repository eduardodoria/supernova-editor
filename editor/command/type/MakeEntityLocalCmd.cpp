#include "MakeEntityLocalCmd.h"

using namespace Supernova;

Editor::MakeEntityLocalCmd::MakeEntityLocalCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->parent = parent;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::MakeEntityLocalCmd::execute(){

    recovery = project->removeEntityFromSharedGroup(sceneId, entity, false);

    return true;
}

void Editor::MakeEntityLocalCmd::undo(){

    project->addEntityToSharedGroup(sceneId, recovery, parent, false);

}

bool Editor::MakeEntityLocalCmd::mergeWith(Editor::Command* otherCommand){

    return false;
}