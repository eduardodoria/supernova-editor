#include "CreateEntityCmd.h"

#include "editor/Log.h"

using namespace Supernova;

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = NULL_ENTITY;
    this->type = EntityCreationType::EMPTY;
}

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = parent;
    this->type = type;
}

void Editor::CreateEntityCmd::execute(){
    std::vector<Supernova::Editor::SceneProject> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            if (entity == NULL_ENTITY){
                entity = scenes[i].scene->createEntity();
            }else{
                entity = scenes[i].scene->createEntityInternal(entity); // recreate same entity
            }

            unsigned int nameCount = 2;
            std::string baseName = entityName;
            bool foundName = true;
            while (foundName){
                foundName = false;
                for (auto& entity : scenes[i].entities) {
                    std::string usedName = scenes[i].scene->getEntityName(entity);
                    if (usedName == entityName){
                        entityName = baseName + " " + std::to_string(nameCount);
                        nameCount++;
                        foundName = true;
                    }
                }
            }

            if (type == EntityCreationType::OBJECT){

                scenes[i].scene->addComponent<Transform>(entity, {});

            }else if (type == EntityCreationType::BOX){

                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<MeshComponent>(entity, {});

                MeshComponent& mesh = scenes[i].scene->getComponent<MeshComponent>(entity);

                scenes[i].scene->getSystem<MeshSystem>()->createBox(entity, 1, 1, 1);

            }else if (type == EntityCreationType::PLANE){

                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<MeshComponent>(entity, {});

                MeshComponent& mesh = scenes[i].scene->getComponent<MeshComponent>(entity);

                scenes[i].scene->getSystem<MeshSystem>()->createPlane(entity, 10, 10);

            }

            scenes[i].scene->setEntityName(entity, entityName);

            if (parent != NULL_ENTITY){
                scenes[i].scene->addEntityChild(parent, entity, false);
            }

            scenes[i].entities.push_back(entity);

            lastSelected = project->getSelectedEntities(sceneId);
            project->setSelectedEntity(sceneId, entity);

            Editor::Log::info("Created entity %s at scene %s", entityName.c_str(), scenes[i].name.c_str());
        }
    }
}

void Editor::CreateEntityCmd::undo(){
    std::vector<Supernova::Editor::SceneProject> &scenes = project->getScenes();
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            scenes[i].scene->destroyEntity(entity);

            auto it = std::find(scenes[i].entities.begin(), scenes[i].entities.end(), entity);
            if (it != scenes[i].entities.end()) {
                scenes[i].entities.erase(it);
            }

            if (project->isSelectedEntity(sceneId, entity)){
                project->replaceSelectedEntities(sceneId, lastSelected);
            }
        }
    }
}

bool Editor::CreateEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}