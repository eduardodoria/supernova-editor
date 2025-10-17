#include "MoveEntityOrderCmd.h"

#include "Out.h"
#include "util/ProjectUtils.h"

using namespace Supernova;        
        
Editor::MoveEntityOrderCmd::MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type){
    this->project = project;
    this->sceneId = sceneId;
    this->source = source;
    this->target = target;
    this->type = type;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::MoveEntityOrderCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (project->isEntityShared(sceneId, source)){

        fs::path sourceSharedPath = project->findGroupPathFor(sceneId, source);
        fs::path targetSharedPath = project->findGroupPathFor(sceneId, target);

        if (type == InsertionType::INTO){
            if (!project->isEntityShared(sceneId, target)){
                Out::error("Cannot move shared entity %u into target %u", source, target);
                return false;
            }
        }else{
            Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);
            if (transformTarget){
                fs::path parentSharedPath = project->findGroupPathFor(sceneId, transformTarget->parent);

                SharedGroup* sourceSourceGroup = project->getSharedGroup(sourceSharedPath);
                uint32_t souceInstanceId = sourceSourceGroup->getInstanceId(sceneId, source);

                bool isSourceRoot = sourceSourceGroup && (sourceSourceGroup->getRootEntity(sceneId, souceInstanceId) == source);

                if (parentSharedPath != sourceSharedPath && !isSourceRoot){
                    Out::error("Cannot move shared entity %u outside shared group", source);
                    return false;
                }
            }
        }

        if (targetSharedPath == sourceSharedPath){
            sharedMoveRecovery = project->moveEntityFromSharedGroup(sceneId, source, target, type, false);
        }
    }
    ProjectUtils::moveEntityOrderByTarget(sceneProject->scene, sceneProject->entities, source, target, type, oldParent, oldIndex, hasTransform);

    sceneProject->isModified = true;

    return true;
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (sharedMoveRecovery.size() > 0){
        project->undoMoveEntityInSharedGroup(sceneId, source, target, sharedMoveRecovery, false);
    }
    ProjectUtils::moveEntityOrderByIndex(sceneProject->scene, sceneProject->entities, source, oldParent, oldIndex, hasTransform);

    sceneProject->isModified = wasModified;
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}