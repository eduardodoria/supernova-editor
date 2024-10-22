#include "DeleteEntityCmd.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->wasSelected = false;
}

void Editor::DeleteEntityCmd::execute(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            entityName = scenes[i].scene->getEntityName(entity);
            signature = scenes[i].scene->getSignature(entity);

            if (signature.test(scenes[i].scene->getComponentType<Transform>())){
                transform = scenes[i].scene->getComponent<Transform>(entity);
            }
            if (signature.test(scenes[i].scene->getComponentType<MeshComponent>())){
                mesh = scenes[i].scene->getComponent<MeshComponent>(entity);
            }

            scenes[i].scene->destroyEntity(entity);

            auto it = std::find(scenes[i].entities.begin(), scenes[i].entities.end(), entity);
            if (it != scenes[i].entities.end()) {
                scenes[i].entities.erase(it);
            }

            if (project->getSelectedEntity(sceneId) == entity){
                project->setSelectedEntity(sceneId, NULL_ENTITY);
                wasSelected = true;
            }
        }
    }
}

void Editor::DeleteEntityCmd::undo(){
    std::vector<Supernova::Editor::SceneData> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            entity = scenes[i].scene->createEntityInternal(entity);
            scenes[i].scene->setEntityName(entity, entityName);

            scenes[i].entities.push_back(entity);

            if (signature.test(scenes[i].scene->getComponentType<Transform>())){
                scenes[i].scene->addComponent<Transform>(entity, transform);
            }
            if (signature.test(scenes[i].scene->getComponentType<MeshComponent>())){
                scenes[i].scene->addComponent<MeshComponent>(entity, {});
            }

            if (wasSelected){
                project->setSelectedEntity(sceneId, entity);
            }
        }
    }
}

bool Editor::DeleteEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}