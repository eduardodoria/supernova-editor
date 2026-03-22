#include "DeleteEntityCmd.h"

#include "Stream.h"
#include "Out.h"
#include "util/ProjectUtils.h"
#include "command/type/MoveEntityOrderCmd.h"
#include "subsystem/MeshSystem.h"
#include "subsystem/ActionSystem.h"

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
    if (registry->isEntityCreated(entity)){ // locked child are deleted by systems when their parent is deleted
        registry->destroyEntity(entity);
    }

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

    auto it = entities.begin();
    while (it != entities.end()) {
        if (ProjectUtils::isEntityLocked(sceneProject->scene, it->entity)){
             Out::warning("Cannot delete entity '%u'. It is a locked child of another component.", it->entity);
             it = entities.erase(it);
        } else {
             ++it;
        }
    }

    if (entities.empty()){
        return false;
    }

    lastSelected = project->getSelectedEntities(sceneId);

    for (DeleteEntityData& entityData : entities){
        // Check if entity is part of a bundle BEFORE encoding (non-root members produce empty nodes)
        fs::path bundlePath = project->findEntityBundlePathFor(sceneId, entityData.entity);
        EntityBundle* bundle = project->getEntityBundle(bundlePath);

        if (bundle) {
            entityData.instanceId = bundle->getInstanceId(sceneId, entityData.entity);

            if (entityData.entity == bundle->getRootEntity(sceneId, entityData.entity)) {
                entityData.isBundleRoot = true;
                entityData.data = Stream::encodeEntity(entityData.entity, sceneProject->scene, project, sceneProject);
                const auto* inst = bundle->getInstance(sceneId, entityData.entity);
                std::vector<Entity> memberEntities;
                if (inst) {
                    for (const auto& member : inst->members) {
                        memberEntities.push_back(member.localEntity);
                    }
                }
                project->unimportEntityBundle(sceneId, bundlePath, entityData.entity, memberEntities);
            } else {
                if (!entityData.hasTransform) {
                    entityData.parent = bundle->getRootEntity(sceneId, entityData.entity);
                }
                entityData.recoveryBundleData = project->removeEntityFromBundle(sceneId, entityData.entity, true);
            }
        } else {
            // Handle model-owned non-transform entities (animations + action tracks)
            Signature sig = sceneProject->scene->getSignature(entityData.entity);
            if (sig.test(sceneProject->scene->getComponentId<ModelComponent>())) {
                ModelComponent& model = sceneProject->scene->getComponent<ModelComponent>(entityData.entity);

                for (Entity animEntity : model.animations) {
                    AnimationComponent* animComp = sceneProject->scene->findComponent<AnimationComponent>(animEntity);
                    if (animComp) {
                        // Encode and destroy action tracks first
                        for (const auto& frame : animComp->actions) {
                            if (sceneProject->scene->isEntityCreated(frame.action)) {
                                DeleteEntityData trackData;
                                trackData.entity = frame.action;
                                trackData.hasTransform = false;
                                auto itTrack = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), frame.action);
                                if (itTrack != sceneProject->entities.end()) {
                                    trackData.entityIndex = std::distance(sceneProject->entities.begin(), itTrack);
                                }
                                trackData.data = Stream::encodeEntity(frame.action, sceneProject->scene, project, sceneProject);
                                entityData.modelOwnedEntities.push_back(trackData);
                                destroyEntity(sceneProject->scene, frame.action, sceneProject->entities, project, sceneId);
                            }
                        }
                    }

                    // Encode animation entity (while ownedActions is still true for correct encoding)
                    if (sceneProject->scene->isEntityCreated(animEntity)) {
                        DeleteEntityData animData;
                        animData.entity = animEntity;
                        animData.hasTransform = false;
                        auto itAnim = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), animEntity);
                        if (itAnim != sceneProject->entities.end()) {
                            animData.entityIndex = std::distance(sceneProject->entities.begin(), itAnim);
                        }
                        animData.data = Stream::encodeEntity(animEntity, sceneProject->scene, project, sceneProject);
                        entityData.modelOwnedEntities.push_back(animData);

                        // Prevent animationDestroy from double-destroying already-destroyed tracks
                        if (animComp) {
                            animComp->ownedActions = false;
                        }
                        destroyEntity(sceneProject->scene, animEntity, sceneProject->entities, project, sceneId);
                    }
                }
            }

            entityData.data = Stream::encodeEntity(entityData.entity, sceneProject->scene, project, sceneProject);

            std::vector<Entity> allEntities;
            project->collectEntities(entityData.data, allEntities);

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
        if (entityData.recoveryBundleData.size() == 0){

            std::vector<Entity> allEntities = Stream::decodeEntity(entityData.data, sceneProject->scene, &sceneProject->entities, project, sceneProject);
            entityData.entity = allEntities[0];

            ProjectUtils::moveEntityOrderByIndex(sceneProject->scene, sceneProject->entities, entityData.entity, entityData.parent, entityData.entityIndex, entityData.hasTransform);

            // Recreate model-owned non-transform entities (action tracks + animations)
            for (DeleteEntityData& ownedEntity : entityData.modelOwnedEntities) {
                std::vector<Entity> ownedEntities = Stream::decodeEntity(ownedEntity.data, sceneProject->scene, &sceneProject->entities, project, sceneProject);
                ownedEntity.entity = ownedEntities[0];
                ProjectUtils::moveEntityOrderByIndex(sceneProject->scene, sceneProject->entities, ownedEntity.entity, ownedEntity.parent, ownedEntity.entityIndex, ownedEntity.hasTransform);
            }

        }else{

            project->addEntityToBundle(sceneId, entityData.recoveryBundleData, entityData.parent, true);
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