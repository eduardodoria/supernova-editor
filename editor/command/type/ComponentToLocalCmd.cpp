#include "ComponentToLocalCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::ComponentToLocalCmd::ComponentToLocalCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->componentType = componentType;
}

bool Editor::ComponentToLocalCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    fs::path filepath = project->findGroupPathFor(sceneId, entity);
    if (sceneProject && !filepath.empty()) {
        SharedGroup* group = project->getSharedGroup(filepath);

        group->setComponentOverride(sceneProject->id, entity, componentType);

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::ComponentToLocalCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    fs::path filepath = project->findGroupPathFor(sceneId, entity);
    if (sceneProject && !filepath.empty()) {
        SharedGroup* group = project->getSharedGroup(filepath);

        group->clearComponentOverride(sceneProject->id, entity, componentType);

        Entity registryEntity = group->getRegistryEntity(sceneId, entity);
        Catalog::copyComponent(group->registry.get(), registryEntity, sceneProject->scene, entity, componentType);

        sceneProject->isModified = true;
    }
}

bool Editor::ComponentToLocalCmd::mergeWith(Command* otherCommand){

    return false;
}