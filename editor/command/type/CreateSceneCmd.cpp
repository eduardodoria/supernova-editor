#include "CreateSceneCmd.h"

using namespace Supernova;

Editor::CreateSceneCmd::CreateSceneCmd(Project* project, std::string sceneName){
    this->project = project;
    this->sceneName = sceneName;
}

void Editor::CreateSceneCmd::execute(){
    unsigned int nameCount = 2;
    std::string baseName = sceneName;
    bool foundName = true;
    while (foundName){
        foundName = false;
        for (auto& sceneProject : project->getScenes()) {
            std::string usedName = sceneProject.name;
            if (usedName == sceneName){
                sceneName = baseName + " " + std::to_string(nameCount);
                nameCount++;
                foundName = true;
            }
        }
    }
}

void Editor::CreateSceneCmd::undo(){

}

bool Editor::CreateSceneCmd::mergeWith(Command* otherCommand){
    return false;
}