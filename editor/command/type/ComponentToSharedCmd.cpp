#include "ComponentToSharedCmd.h"

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
        //Scene* scene = sceneProject->scene;
        SharedGroup* group = project->getSharedGroup(filepath);

        //recovery = Project::removeEntityComponent(scene, entity, componentType, sceneProject->entities, true);

        // Clear the override and copy values from the shared registry
        group->clearComponentOverride(sceneProject->id, entity, componentType);

        Entity registryEntity = group->getRegistryEntity(sceneId, entity);

        // Copy all properties from registry to scene entity
        auto props = Catalog::getProperties(componentType, nullptr);
        for (const auto& [propId, propData] : props) {
            Catalog::copyPropertyValue(group->registry.get(), registryEntity, sceneProject->scene, entity, componentType, propId);
        }

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

        sceneProject->isModified = true;
    }
}

bool Editor::ComponentToSharedCmd::mergeWith(Command* otherCommand){

    return false;
}