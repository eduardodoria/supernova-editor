#include "RemoveComponentCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::RemoveComponentCmd::RemoveComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->componentType = componentType;
}

bool Editor::RemoveComponentCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        fs::path filepath = project->findGroupPathFor(sceneId, entity);
        SharedGroup* group = project->getSharedGroup(filepath);

        if (group && !group->hasComponentOverride(sceneId, entity, componentType)){

            recovery = project->removeComponentToSharedGroup(sceneId, entity, componentType, true, true);

        }else{

            oldComponent = Project::removeEntityComponent(scene, entity, componentType, sceneProject->entities, true);

            if (group){
                hasOverride = group->hasComponentOverride(sceneId, entity, componentType);
                if (hasOverride){
                    group->clearComponentOverride(sceneId, entity, componentType);
                }
            }

        }

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::RemoveComponentCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        if (recovery.size() > 0){

            project->addComponentToSharedGroup(sceneId, entity, componentType, recovery, true);

        }else{

            Project::addEntityComponent(scene, entity, componentType, sceneProject->entities, oldComponent);

            if (hasOverride){
                fs::path filepath = project->findGroupPathFor(sceneId, entity);
                SharedGroup* group = project->getSharedGroup(filepath);

                if (group){
                    group->setComponentOverride(sceneId, entity, componentType);
                }
            }

        }

        sceneProject->isModified = true;
    }
}

bool Editor::RemoveComponentCmd::mergeWith(Command* otherCommand){

    return false;
}