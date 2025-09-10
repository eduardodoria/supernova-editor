#include "ComponentToSharedCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::ComponentToSharedCmd::ComponentToSharedCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->componentType = componentType;
}

bool Editor::ComponentToSharedCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    fs::path filepath = project->findGroupPathFor(sceneId, entity);
    if (sceneProject && !filepath.empty()) {
        SharedGroup* group = project->getSharedGroup(filepath);

        Signature signature = Catalog::componentTypeToSignature(sceneProject->scene, componentType);
        recovery = Stream::encodeComponents(entity, sceneProject->scene, signature);

        // Clear the override and copy values from the shared registry
        group->clearComponentOverride(sceneProject->id, entity, componentType);

        Entity registryEntity = group->getRegistryEntity(sceneId, entity);
        Catalog::copyComponent(group->registry.get(), registryEntity, sceneProject->scene, entity, componentType);

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::ComponentToSharedCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    fs::path filepath = project->findGroupPathFor(sceneId, entity);
    if (sceneProject && !filepath.empty()) {
        SharedGroup* group = project->getSharedGroup(filepath);

        group->setComponentOverride(sceneProject->id, entity, componentType);

        Entity parent = NULL_ENTITY;
        if (componentType == ComponentType::Transform){
            parent = sceneProject->scene->getComponent<Transform>(entity).parent;
        }

        Stream::decodeComponents(entity, parent, sceneProject->scene, recovery);

        sceneProject->isModified = true;
    }
}

bool Editor::ComponentToSharedCmd::mergeWith(Command* otherCommand){

    return false;
}