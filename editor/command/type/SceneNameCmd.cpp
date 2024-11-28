#include "SceneNameCmd.h"

using namespace Supernova;

Editor::SceneNameCmd::SceneNameCmd(Project* project, uint32_t sceneId, std::string name){
    this->project = project;
    this->sceneId = sceneId;
    this->newName = name;
}

void Editor::SceneNameCmd::execute(){
    oldName = project->getScene(sceneId)->name;
    project->getScene(sceneId)->name = newName;
}

void Editor::SceneNameCmd::undo(){
    project->getScene(sceneId)->name = oldName;
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