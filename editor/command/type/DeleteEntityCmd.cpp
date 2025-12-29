#include "DeleteEntityCmd.h"

#include "Stream.h"
#include "Out.h"
#include "util/ProjectUtils.h"
#include "command/type/MoveEntityOrderCmd.h"

using namespace Supernova;

Editor::DeleteEntityCmd::DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity){
    this->project = project;
    this->sceneId = sceneId;

    DeleteEntityData entityData;
    entityData.entity = entity;

    SceneProject* sceneProject = project->getScene(sceneId);
    Scene* scene = sceneProject->instance.scene;

    Signature signature = scene->getSignature(entity);
    if (signature.test(scene->getComponentId<Transform>())) {
        entityData.hasTransform = true;
        Transform& transform = scene->getComponent<Transform>(entity);
        auto transforms = scene->getComponentArray<Transform>();
        entityData.entityIndex = transforms->getIndex(entity);
        entityData.parent = transform.parent;
    }else{
        entityData.hasTransform = false;
        auto it = std::find(sceneProject->instance.entities.begin(), sceneProject->instance.entities.end(), entity);
        if (it != sceneProject->instance.entities.end()) {
            entityData.entityIndex = std::distance(sceneProject->instance.entities.begin(), it);
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

        SceneProject* sceneProject = project->getScene(sceneId);
        if (sceneProject && sceneProject->mainCamera == entity){
            sceneProject->mainCamera = NULL_ENTITY;
        }
    }
}

bool Editor::DeleteEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    lastSelected = project->getSelectedEntities(sceneId);

    for (DeleteEntityData& entityData : entities){
        entityData.data = Stream::encodeEntity(entityData.entity, sceneProject->instance.scene, project, sceneProject);

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

                destroyEntity(sceneProject->instance.scene, entity, sceneProject->instance.entities, project, sceneId);

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

            std::vector<Entity> allEntities = Stream::decodeEntity(entityData.data, sceneProject->instance.scene, &sceneProject->instance.entities, project, sceneProject);
            entityData.entity = allEntities[0];

            ProjectUtils::moveEntityOrderByIndex(sceneProject->instance.scene, sceneProject->instance.entities, entityData.entity, entityData.parent, entityData.entityIndex, entityData.hasTransform);

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