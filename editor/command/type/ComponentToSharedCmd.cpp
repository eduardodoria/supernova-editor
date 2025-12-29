#include "ComponentToSharedCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::ComponentToSharedCmd::ComponentToSharedCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->componentType = componentType;

    ComponentToSharedData entityData;
    entityData.entity = entity;
    entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::ComponentToSharedCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);

    for (ComponentToSharedData& entityData : entities){

        fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
        if (sceneProject && !filepath.empty()) {
            SharedGroup* group = project->getSharedGroup(filepath);

            Signature signature = Catalog::componentTypeToSignature(sceneProject->instance.scene, componentType);
            entityData.recovery = Stream::encodeComponents(entityData.entity, sceneProject->instance.scene, signature);

            if (!group->hasComponentOverride(sceneId, entityData.entity, componentType)){
                return false;
            }

            // Clear the override and copy values from the shared registry
            group->clearComponentOverride(sceneProject->id, entityData.entity, componentType);

            Entity registryEntity = group->getRegistryEntity(sceneId, entityData.entity);
            Catalog::copyComponent(group->registry.get(), registryEntity, sceneProject->instance.scene, entityData.entity, componentType);
        }

    }

    sceneProject->isModified = true;

    return true;
}

void Editor::ComponentToSharedCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);

    for (ComponentToSharedData& entityData : entities){

        fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
        if (sceneProject && !filepath.empty()) {
            SharedGroup* group = project->getSharedGroup(filepath);

            group->setComponentOverride(sceneProject->id, entityData.entity, componentType);

            Entity parent = NULL_ENTITY;
            if (componentType == ComponentType::Transform){
                parent = sceneProject->instance.scene->getComponent<Transform>(entityData.entity).parent;
            }

            Stream::decodeComponents(entityData.entity, parent, sceneProject->instance.scene, entityData.recovery);
        }

    }

    sceneProject->isModified = wasModified;
}

bool Editor::ComponentToSharedCmd::mergeWith(Command* otherCommand){
    ComponentToSharedCmd* otherCmd = dynamic_cast<ComponentToSharedCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            for (ComponentToSharedData& otherEntityData :  otherCmd->entities){
                entities.push_back(otherEntityData);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}