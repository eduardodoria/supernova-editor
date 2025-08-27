#include "DeleteEntityCmd.h"

#include "Stream.h"
#include "Out.h"

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
        Transform& transform = scene->getComponent<Transform>(entity);
        auto transforms = scene->getComponentArray<Transform>();
        entityData.transformIndex = transforms->getIndex(entity);
        entityData.parent = transform.parent;
    }

    auto it = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entity);
    if (it != sceneProject->entities.end()) {
        entityData.entityIndex = std::distance(sceneProject->entities.begin(), it);
    }

    this->entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
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

            if (entity == group->getRootEntity(sceneId)) {

                project->unimportSharedEntity(sceneId, sharedGroupPath, sharedEntities, false);

            }else{

                //Entity regEntity = group->getRegistryEntity(sceneId, entity);
                //if (regEntity == NULL_ENTITY) {
                //    Out::error("Failed to find registry entity for shared entity %u in scene %u", entity, sceneId);
                //    continue;
                //}
                //YAML::Node extendNode = YAML::Clone(entityData.data);
                //entityData.data = Stream::encodeEntity(regEntity, group->registry.get());

                //std::vector<size_t> indicesNotShared = project->mergeEntityNodes(entityData.data, extendNode);
                entityData.wasSharedChild = true;
                entityData.sharedGroupPath = sharedGroupPath;
                entityData.sharedData = project->removeEntityFromSharedGroup2(sceneId, entity, sharedGroupPath);

            }
        }
/*
        for (const Entity& entity : allEntities) {
            sceneProject->scene->destroyEntity(entity);

            auto ite = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), entity);
            if (ite != sceneProject->entities.end()) {
                sceneProject->entities.erase(ite);
            }

            if (project->isSelectedEntity(sceneId, entity)){
                project->clearSelectedEntities(sceneId);
            }
        }
*/
        sceneProject->isModified = true;
    }

    return true;
}

void Editor::DeleteEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    for (DeleteEntityData& entityData : entities){
        //std::vector<Entity> allEntities = Stream::decodeEntity(entityData.data, sceneProject->scene, project, sceneProject);
        //entityData.entity = allEntities[0];

        //if (entityData.parent != NULL_ENTITY) {
        //    sceneProject->scene->addEntityChild(entityData.parent, entityData.entity, false);
        //}

        //sceneProject->scene->moveChildToIndex(entityData.entity, entityData.transformIndex, false);

        if (entityData.wasSharedChild){
            project->addEntityToSharedGroup2(sceneId, entityData.sharedData, entityData.parent, entityData.sharedGroupPath);
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
                // insert at begin to keep deletion order
                entities.push_back(otherEntityData);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}