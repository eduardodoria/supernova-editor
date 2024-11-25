#include "DeleteEntityCmd.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;

    DeleteEntityData entityData;
    entityData.entity = entity;
    this->entities.push_back(entityData);
}

void Editor::DeleteEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    lastSelected = project->getSelectedEntities(sceneId);

    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        DeleteEntityData& entityData = *it;

        entityData.entityName = sceneProject->scene->getEntityName(entityData.entity);
        entityData.signature = sceneProject->scene->getSignature(entityData.entity);

        if (entityData.signature.test(sceneProject->scene->getComponentId<Transform>())){
            entityData.transform = sceneProject->scene->getComponent<Transform>(entityData.entity);
            entityData.transformIndex = sceneProject->scene->getComponentArray<Transform>()->getIndex(entityData.entity);
            entityData.parent = entityData.transform.parent;
        }
        if (entityData.signature.test(sceneProject->scene->getComponentId<MeshComponent>())){
            entityData.mesh = sceneProject->scene->getComponent<MeshComponent>(entityData.entity);
        }

        sceneProject->scene->destroyEntity(entityData.entity);

        auto ite = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entityData.entity);
        if (ite != sceneProject->entities.end()) {
            sceneProject->entities.erase(ite);
        }

        if (project->isSelectedEntity(sceneId, entityData.entity)){
            project->clearSelectedEntities(sceneId);
        }
    }
}

void Editor::DeleteEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    for (DeleteEntityData& entityData : entities){

        entityData.entity = sceneProject->scene->createEntityInternal(entityData.entity);

        sceneProject->entities.push_back(entityData.entity);

        sceneProject->scene->setEntityName(entityData.entity, entityData.entityName);

        if (entityData.signature.test(sceneProject->scene->getComponentId<Transform>())){
            sceneProject->scene->addComponent<Transform>(entityData.entity, entityData.transform);
            sceneProject->scene->moveChildToIndex(entityData.entity, entityData.transformIndex);
            if (entityData.parent != NULL_ENTITY){
                sceneProject->scene->addEntityChild(entityData.parent, entityData.entity, false);
            }
        }
        if (entityData.signature.test(sceneProject->scene->getComponentId<MeshComponent>())){
            sceneProject->scene->addComponent<MeshComponent>(entityData.entity, {});
        }
    }

    if (lastSelected.size() > 0){
        project->replaceSelectedEntities(sceneId, lastSelected);
    }
}

bool Editor::DeleteEntityCmd::mergeWith(Editor::Command* otherCommand){
    DeleteEntityCmd* otherCmd = dynamic_cast<DeleteEntityCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            lastSelected = otherCmd->lastSelected;

            for (DeleteEntityData& otherEntityData :  otherCmd->entities){
                // insert at begin to keep deletion order
                entities.push_back(otherEntityData);
            }

            std::sort(entities.begin(), entities.end(), [](const DeleteEntityData& a, const DeleteEntityData& b) {
                return a.parent < b.parent || a.transformIndex < b.transformIndex;
            });

            return true;
        }
    }

    return false;
}