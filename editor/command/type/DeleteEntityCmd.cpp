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

void Editor::DeleteEntityCmd::collectEntities(const YAML::Node& entityNode, std::vector<Entity>& entities) {
    if (!entityNode || !entityNode.IsMap())
        return;

    if (entityNode["entity"]) {
        entities.push_back(entityNode["entity"].as<Entity>());
    }

    // Recursively process children
    if (entityNode["children"] && entityNode["children"].IsSequence()) {
        for (const auto& child : entityNode["children"]) {
            collectEntities(child, entities);
        }
    }
}

bool Editor::DeleteEntityCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    lastSelected = project->getSelectedEntities(sceneId);

    for (DeleteEntityData& entityData : entities){
        entityData.data = Stream::encodeEntityBranch(entityData.entity, sceneProject, true);

        std::vector<Entity> allEntities;
        collectEntities(entityData.data, allEntities);

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

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::DeleteEntityCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    for (DeleteEntityData& entityData : entities){
        std::vector<Entity> allEntities = Stream::decodeEntity(sceneProject->scene, entityData.data);
        entityData.entity = allEntities[0];

        if (entityData.parent != NULL_ENTITY) {
            sceneProject->scene->addEntityChild(entityData.parent, entityData.entity, false);
        }

        sceneProject->entities.insert(sceneProject->entities.begin() + entityData.entityIndex, allEntities.begin(), allEntities.end());

        sceneProject->scene->moveChildToIndex(entityData.entity, entityData.transformIndex, false);
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