#include "MakeEntitySharedCmd.h"

using namespace Supernova;

Editor::MakeEntitySharedCmd::MakeEntitySharedCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->parent = parent;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::MakeEntitySharedCmd::execute(){

    project->addEntityToSharedGroup(sceneId, entity, parent, false);

    return true;
}

void Editor::MakeEntitySharedCmd::undo(){

    project->removeEntityFromSharedGroup(sceneId, entity, false);

}

bool Editor::MakeEntitySharedCmd::mergeWith(Editor::Command* otherCommand){

    return false;
}