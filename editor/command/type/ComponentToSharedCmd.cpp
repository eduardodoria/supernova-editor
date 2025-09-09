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
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        //Stream::encodeCo

        fs::path filepath = project->findGroupPathFor(sceneId, entity);
        SharedGroup* sharedGroup = project->getSharedGroup(filepath);

        recovery = Project::removeEntityComponent(scene, entity, componentType, sceneProject->entities, true);

        // Clear the override and copy values from the shared registry
        sharedGroup->clearComponentOverride(sceneProject->id, entity, componentType);

        // Find entity index in the shared group
        const auto& entities = sharedGroup->getAllEntities(sceneProject->id);
        auto it = std::find(entities.begin(), entities.end(), entity);
        if (it != entities.end()) {
            size_t entityIndex = std::distance(entities.begin(), it);
            Entity registryEntity = entityIndex + NULL_ENTITY + 1;

            // Copy all properties from registry to scene entity
            auto props = Catalog::getProperties(componentType, nullptr);
            for (const auto& [propId, propData] : props) {
                Catalog::copyPropertyValue(sharedGroup->registry.get(), registryEntity, sceneProject->scene, entity, componentType, propId);
            }
        }

        sceneProject->isModified = true;

        //Project::addEntityComponent(scene, entity, componentType, sceneProject->entities);

        //if (project->isEntityShared(sceneId, entity)){
        //    project->addComponentToSharedGroup(sceneId, entity, componentType, false);
        //}
    }

    return true;
}

void Editor::ComponentToSharedCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        //Project::removeEntityComponent(scene, entity, componentType, sceneProject->entities);

        //if (project->isEntityShared(sceneId, entity)){
        //    project->removeComponentToSharedGroup(sceneId, entity, componentType, false, false);
        //}

        sceneProject->isModified = true;
    }
}

bool Editor::ComponentToSharedCmd::mergeWith(Command* otherCommand){

    return false;
}