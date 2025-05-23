#include "CreateEntityCmd.h"

#include "editor/Out.h"

using namespace Supernova;

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = NULL_ENTITY;
    this->type = EntityCreationType::EMPTY;
    this->updateFlags = 0;
}

Editor::CreateEntityCmd::CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type, Entity parent){
    this->project = project;
    this->sceneId = sceneId;
    this->entityName = entityName;
    this->entity = NULL_ENTITY;
    this->parent = parent;
    this->type = type;
    this->updateFlags = 0;
}

bool Editor::CreateEntityCmd::execute(){
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

                scenes[i].scene->getSystem<MeshSystem>()->createBox(entity, 1, 1, 1);

            }else if (type == EntityCreationType::PLANE){

                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<MeshComponent>(entity, {});

                scenes[i].scene->getSystem<MeshSystem>()->createPlane(entity, 10, 10);

            }else if (type == EntityCreationType::IMAGE){

                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<UILayoutComponent>(entity, {});
                scenes[i].scene->addComponent<UIComponent>(entity, {});
                scenes[i].scene->addComponent<ImageComponent>(entity, {});

                UILayoutComponent& layout = scenes[i].scene->getComponent<UILayoutComponent>(entity);
                layout.width = 100;
                layout.height = 100;
            }else if (type == EntityCreationType::SPRITE){

                scenes[i].scene->addComponent<Transform>(entity, {});
                scenes[i].scene->addComponent<MeshComponent>(entity, {});
                scenes[i].scene->addComponent<SpriteComponent>(entity, {});

                SpriteComponent& sprite = scenes[i].scene->getComponent<SpriteComponent>(entity);
                sprite.width = 100;
                sprite.height = 100;
            }

            scenes[i].scene->setEntityName(entity, entityName);

            if (parent != NULL_ENTITY){
                scenes[i].scene->addEntityChild(parent, entity, false);
            }

            // Apply all property setters
            for (const auto& [componentType, properties] : propertySetters) {
                for (const auto& [propertyName, propertySetter] : properties) {
                    propertySetter(entity);
                }
            }

            Catalog::updateEntity(scenes[i].scene, entity, updateFlags);

            scenes[i].entities.push_back(entity);

            lastSelected = project->getSelectedEntities(sceneId);
            project->setSelectedEntity(sceneId, entity);

            scenes[i].isModified = true;

            ImGui::SetWindowFocus(("###Scene" + std::to_string(sceneId)).c_str());

            Editor::Out::info("Created entity '%s' at scene '%s'", entityName.c_str(), scenes[i].name.c_str());
        }
    }

    return true;
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

            scenes[i].isModified = true;
        }
    }
}

bool Editor::CreateEntityCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}

Entity Editor::CreateEntityCmd::getEntity(){
    return entity;
}