#include "DeleteEntityCmd.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
}

void Editor::DeleteEntityCmd::execute(){
    std::vector<Supernova::Editor::SceneProject> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            entityName = scenes[i].scene->getEntityName(entity);
            signature = scenes[i].scene->getSignature(entity);

            if (signature.test(scenes[i].scene->getComponentId<Transform>())){
                transform = scenes[i].scene->getComponent<Transform>(entity);
                parent = transform.parent;
            }
            if (signature.test(scenes[i].scene->getComponentId<MeshComponent>())){
                mesh = scenes[i].scene->getComponent<MeshComponent>(entity);
            }

            scenes[i].scene->destroyEntity(entity);

            auto it = std::find(scenes[i].entities.begin(), scenes[i].entities.end(), entity);
            if (it != scenes[i].entities.end()) {
                scenes[i].entities.erase(it);
            }

            lastSelected = project->getSelectedEntities(sceneId);

            if (project->isSelectedEntity(sceneId, entity)){
                project->clearSelectedEntities(sceneId);
            }
        }
    }
}

void Editor::DeleteEntityCmd::undo(){
    std::vector<Supernova::Editor::SceneProject> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            entity = scenes[i].scene->createEntityInternal(entity);

            if (signature.test(scenes[i].scene->getComponentId<Transform>())){
                scenes[i].scene->addComponent<Transform>(entity, transform);
            }
            if (signature.test(scenes[i].scene->getComponentId<MeshComponent>())){
                scenes[i].scene->addComponent<MeshComponent>(entity, {});
            }

            scenes[i].scene->setEntityName(entity, entityName);
            if (parent != NULL_ENTITY){
                scenes[i].scene->addEntityChild(parent, entity, false);
            }

            scenes[i].entities.push_back(entity);

            if (lastSelected.size() > 0){
                project->replaceSelectedEntities(sceneId, lastSelected);
            }
        }
    }
}

bool Editor::DeleteEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}