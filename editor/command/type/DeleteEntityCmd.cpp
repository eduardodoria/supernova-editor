#include "DeleteEntityCmd.h"

#include "Stream.h"
#include "Out.h"
#include "command/type/MoveEntityOrderCmd.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;

    DeleteEntityData entityData;
    entityData.entity = entity;

    SceneProject* sceneProject = project->getScene(sceneId);
    Scene* scene = sceneProject->scene;

    Signature signature = scene->getSignature(entity);
    if (signature.test(scene->getComponentId<Transform>())) {
        entityData.hasTransform = true;
        Transform& transform = scene->getComponent<Transform>(entity);
        auto transforms = scene->getComponentArray<Transform>();
        entityData.entityIndex = transforms->getIndex(entity);
        entityData.parent = transform.parent;
    }else{
        entityData.hasTransform = false;
        auto it = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entity);
        if (it != sceneProject->entities.end()) {
            entityData.entityIndex = std::distance(sceneProject->entities.begin(), it);
        }
    }

    this->entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
}

void Editor::DeleteEntityCmd::destroyEntity(EntityRegistry* registry, Entity entity, std::vector<Entity>& entities, Project* project, uint32_t sceneId){
    registry->destroyEntity(entity);

    auto ite = std::find(entities.begin(), entities.end(), entity);
    if (ite != entities.end()) {
        entities.erase(ite);
    }

    if (project){
        if (project->isSelectedEntity(sceneId, entity)){
            project->clearSelectedEntities(sceneId);
        }
    }
}

bool Editor::DeleteEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    lastSelected = project->getSelectedEntities(sceneId);

    for (DeleteEntityData& entityData : entities){
        entityData.data = Stream::encodeEntity(entityData.entity, sceneProject->scene, project, sceneProject, true);

        std::vector<Entity> allEntities;
        std::vector<Entity> sharedEntities;
        project->collectEntities(entityData.data, allEntities, sharedEntities);

        if (sharedEntities.size() > 0) {
            Entity entity = sharedEntities[0];

            fs::path sharedGroupPath = project->findGroupPathFor(sceneId, entity);
            SharedGroup* group = project->getSharedGroup(sharedGroupPath);
            entityData.instanceId = group->getInstanceId(sceneId, entity);

            if (entity == group->getRootEntity(sceneId, entityData.instanceId)) {

                project->unimportSharedEntity(sceneId, sharedGroupPath, sharedEntities, false);

            }else{

                entityData.recoverySharedData = project->removeEntityFromSharedGroup(sceneId, entity, true);

            }
        }

        if (entityData.recoverySharedData.size() == 0){
            for (const Entity& entity : allEntities) {

                destroyEntity(sceneProject->scene, entity, sceneProject->entities, project, sceneId);

            }
        }

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::DeleteEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    for (DeleteEntityData& entityData : entities){
        if (entityData.recoverySharedData.size() == 0){

            std::vector<Entity> allEntities = Stream::decodeEntity(entityData.data, sceneProject->scene, &sceneProject->entities, project, sceneProject);
            entityData.entity = allEntities[0];

            MoveEntityOrderCmd::moveEntityOrderByIndex(sceneProject->scene, sceneProject->entities, entityData.entity, entityData.parent, entityData.entityIndex, entityData.hasTransform);

        }else{

            project->addEntityToSharedGroup(sceneId, entityData.recoverySharedData, entityData.parent, entityData.instanceId, true);

        }
    }

    if (lastSelected.size() > 0){
        project->replaceSelectedEntities(sceneId, lastSelected);
    }

    sceneProject->isModified = wasModified;
}

bool Editor::DeleteEntityCmd::mergeWith(Editor::Command* otherCommand){
    DeleteEntityCmd* otherCmd = dynamic_cast<DeleteEntityCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            lastSelected = otherCmd->lastSelected;

            for (DeleteEntityData& otherEntityData :  otherCmd->entities){
                entities.push_back(otherEntityData);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}