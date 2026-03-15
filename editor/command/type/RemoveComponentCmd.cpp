#include "RemoveComponentCmd.h"

#include "Stream.h"
#include "Out.h"
#include "util/ProjectUtils.h"

using namespace Supernova;

Editor::RemoveComponentCmd::RemoveComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->componentType = componentType;

    RemoveComponentData entityData;
    entityData.entity = entity;

    this->entities.push_back(entityData);

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::RemoveComponentCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        for (RemoveComponentData& entityData : entities){

            fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
            SharedGroup* group = project->getSharedGroup(filepath);

            fs::path bundlePath = project->findEntityBundlePathFor(sceneId, entityData.entity);
            EntityBundle* bundle = project->getEntityBundle(bundlePath);

            if (group && componentType == ComponentType::Transform){
                Out::error("Cannot remove Transform component from shared entity '%s' in scene '%s'", scene->getEntityName(entityData.entity).c_str(), sceneProject->name.c_str());
                continue;
            }

            if (bundle && componentType == ComponentType::Transform){
                Out::error("Cannot remove Transform component from bundle entity '%s' in scene '%s'", scene->getEntityName(entityData.entity).c_str(), sceneProject->name.c_str());
                continue;
            }

            if (group && !group->hasComponentOverride(sceneId, entityData.entity, componentType)){

                entityData.recovery = project->removeComponentToSharedGroup(sceneId, entityData.entity, componentType, true, true);

            }else if (bundle && !bundle->hasComponentOverride(sceneId, entityData.entity, componentType)){

                entityData.recovery = project->removeComponentFromBundle(sceneId, entityData.entity, componentType, true, true);

            }else{

                entityData.oldComponent = ProjectUtils::removeEntityComponent(scene, entityData.entity, componentType, sceneProject->entities, true);

                if (group){
                    entityData.hasOverride = group->hasComponentOverride(sceneId, entityData.entity, componentType);
                    if (entityData.hasOverride){
                        group->clearComponentOverride(sceneId, entityData.entity, componentType);
                    }
                }else if (bundle){
                    entityData.hasOverride = bundle->hasComponentOverride(sceneId, entityData.entity, componentType);
                    if (entityData.hasOverride){
                        bundle->clearComponentOverride(sceneId, entityData.entity, componentType);
                    }
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

        for (RemoveComponentData& entityData : entities){
            if (entityData.recovery.size() > 0){

                fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
                if (!filepath.empty()){
                    project->addComponentToSharedGroup(sceneId, entityData.entity, componentType, entityData.recovery, true);
                }else{
                    project->addComponentToBundle(sceneId, entityData.entity, componentType, entityData.recovery, true);
                }

            }else{

                ProjectUtils::addEntityComponent(scene, entityData.entity, componentType, sceneProject->entities, entityData.oldComponent);

                if (entityData.hasOverride){
                    fs::path filepath = project->findGroupPathFor(sceneId, entityData.entity);
                    SharedGroup* group = project->getSharedGroup(filepath);

                    if (group){
                        group->setComponentOverride(sceneId, entityData.entity, componentType);
                    }else{
                        fs::path bundlePath = project->findEntityBundlePathFor(sceneId, entityData.entity);
                        EntityBundle* bundle = project->getEntityBundle(bundlePath);

                        if (bundle){
                            bundle->setComponentOverride(sceneId, entityData.entity, componentType);
                        }
                    }
                }

            }
        }

        sceneProject->isModified = wasModified;
    }
}

bool Editor::RemoveComponentCmd::mergeWith(Command* otherCommand){
    RemoveComponentCmd* otherCmd = dynamic_cast<RemoveComponentCmd*>(otherCommand);
    if (otherCmd != nullptr){
        if (sceneId == otherCmd->sceneId){

            for (RemoveComponentData& otherEntityData :  otherCmd->entities){
                entities.push_back(otherEntityData);
            }

            wasModified = wasModified && otherCmd->wasModified;

            return true;
        }
    }

    return false;
}