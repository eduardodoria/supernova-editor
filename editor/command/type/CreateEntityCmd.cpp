#include "CreateEntityCmd.h"

using namespace Supernova;

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->type = EntityCreationType::EMPTY;
}

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->type = type;
}

void Editor::CreateEntityCmd::execute(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            if (entity == NULL_ENTITY){
                entity = scenes[i].scene->createEntity();
            }else{
                entity = scenes[i].scene->createEntityInternal(entity); // recreate same entity
            }

            if (type == EntityCreationType::BOX){
                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<MeshComponent>(entity, {});

                MeshComponent& mesh = scenes[i].scene->getComponent<MeshComponent>(entity);

                scenes[i].scene->getSystem<MeshSystem>()->createBox(entity, 1, 1, 1);
            }

            scenes[i].scene->setEntityName(entity, entityName);

            scenes[i].entities.push_back(entity);

            project->setSelectedEntity(sceneId, entity);
        }
    }
}

void Editor::CreateEntityCmd::undo(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            scenes[i].scene->destroyEntity(entity);

            auto it = std::find(scenes[i].entities.begin(), scenes[i].entities.end(), entity);
            if (it != scenes[i].entities.end()) {
                scenes[i].entities.erase(it);
            }

            if (project->getSelectedEntity(sceneId) == entity){
                project->setSelectedEntity(sceneId, NULL_ENTITY);
            }
        }
    }
}

bool Editor::CreateEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}