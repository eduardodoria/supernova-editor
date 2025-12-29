#include "ComponentToLocalCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::ComponentToLocalCmd::ComponentToLocalCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entities.push_back(entity);
    this->componentType = componentType;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::ComponentToLocalCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    for (Entity entity : entities){
        fs::path filepath = project->findGroupPathFor(sceneId, entity);
        if (sceneProject && !filepath.empty()) {
            SharedGroup* group = project->getSharedGroup(filepath);

            if (group->hasComponentOverride(sceneId, entity, componentType)){
                return false;
            }

            group->setComponentOverride(sceneProject->id, entity, componentType);
        }
    }

    sceneProject->isModified = true;

    return true;
}

void Editor::ComponentToLocalCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    for (Entity entity : entities){
        fs::path filepath = project->findGroupPathFor(sceneId, entity);
        if (sceneProject && !filepath.empty()) {
            SharedGroup* group = project->getSharedGroup(filepath);

            group->clearComponentOverride(sceneProject->id, entity, componentType);

            Entity registryEntity = group->getRegistryEntity(sceneId, entity);
            Catalog::copyComponent(group->registry.get(), registryEntity, sceneProject->instance.scene, entity, componentType);
        }
    }

    sceneProject->isModified = wasModified;
}

bool Editor::ComponentToLocalCmd::mergeWith(Command* otherCommand){
    ComponentToLocalCmd* otherCmd = dynamic_cast<ComponentToLocalCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            for (Entity& otherEntity :  otherCmd->entities){
                entities.push_back(otherEntity);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}