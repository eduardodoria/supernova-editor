#include "Project.h"

using namespace Supernova;

uint32_t Editor::Project::nextSceneId = 0;

Editor::Project::Project(){
}

void Editor::Project::createNewScene(std::string sceneName){
    scenes.push_back({++nextSceneId, sceneName, new Scene()});
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

bool Editor::Project::createNewComponent(std::string sceneName, Entity entity, ComponentType component){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].name == sceneName){
            if (component == ComponentType::Transform){
                scenes[i].scene->addComponent<Transform>(entity, {});
            }
            return true;
        }
    }

    return false;
}

std::vector<Editor::SceneData>&  Editor::Project::getScenes(){
    return scenes;
}