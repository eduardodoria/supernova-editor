#include "CreateEntityCmd.h"

using namespace Supernova;

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
}

void Editor::CreateEntityCmd::execute(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            Entity entity = scenes[i].scene->createEntity();
            scenes[i].scene->setEntityName(entity, entityName);

            scenes[i].entities.push_back(entity);

            project->setSelectedEntity(sceneId, entity);

            this->entity = entity;
        }
    }
}

void Editor::CreateEntityCmd::undo(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            scenes[i].scene->destroyEntity(entity);

            // Remove the element using erase function and iterators
            auto it = std::find(scenes[i].entities.begin(), scenes[i].entities.end(), entity);

            // If element is found found, erase it
            if (it != scenes[i].entities.end()) {
                scenes[i].entities.erase(it);
            }

            if (project->getSelectedEntity(sceneId) == entity){
                project->setSelectedEntity(sceneId, NULL_ENTITY);
            }

            this->entity = NULL_ENTITY;
        }
    }
}