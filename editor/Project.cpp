#include "Project.h"

using namespace Supernova;

Editor::Project::Project(){
}

void Editor::Project::createNewScene(std::string sceneName){
    scenes.push_back({sceneName, new Scene()});
}

Entity Editor::Project::createNewEntity(std::string sceneName){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].name == sceneName){
            Entity entity = scenes[i].scene->createEntity();
            scenes[i].entities.push_back(entity);

            return entity;
        }
    }

    return NULL_ENTITY;
}

void Editor::Project::createNewComponent(std::string sceneName, Entity entity, ComponentType component){

}
