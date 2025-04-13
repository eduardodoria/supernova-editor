#include "SceneNameCmd.h"

using namespace Supernova;

Editor::SceneNameCmd::SceneNameCmd(Project* project, uint32_t sceneId, std::string name){
    this->project = project;
    this->sceneId = sceneId;
    this->newName = name;
}

bool Editor::SceneNameCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    oldName = sceneProject->name;
    sceneProject->name = newName;
    sceneProject->isModified = true;

    return true;
}

void Editor::SceneNameCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    sceneProject->name = oldName;
    sceneProject->isModified = true;
}

bool Editor::SceneNameCmd::mergeWith(Editor::Command* otherCommand){
    SceneNameCmd* otherCmd = dynamic_cast<SceneNameCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){
            this->oldName = otherCmd->oldName;
            return true;
        }
    }
    return false;
}