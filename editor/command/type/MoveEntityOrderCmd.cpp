#include "MoveEntityOrderCmd.h"

#include "Out.h"

using namespace Supernova;        
        
Editor::MoveEntityOrderCmd::MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type){
    this->project = project;
    this->sceneId = sceneId;
    this->source = source;
    this->target = target;
    this->type = type;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::MoveEntityOrderCmd::moveEntityOrderByTarget(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, bool& hasTransform) {
    Transform* transformSource = registry->findComponent<Transform>(source);
    Transform* transformTarget = registry->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        hasTransform = true;

        if (registry->isParentOf(source, target)){
            Out::error("Cannot move entity to a child");
            return false;
        }

        auto transforms = registry->getComponentArray<Transform>();

        size_t sourceTransformIndex = transforms->getIndex(source);
        size_t targetTransformIndex = transforms->getIndex(target);

        // Need to be before addEntityChild
        size_t sizeOfSourceBranch = registry->findBranchLastIndex(source) - sourceTransformIndex + 1;
        bool needAdjustBranch = (sourceTransformIndex < targetTransformIndex);

        Entity newParent = NULL_ENTITY;
        if (type == InsertionType::IN){
            newParent = target;
        }else{
            newParent = transformTarget->parent;
        }

        oldParent = transformSource->parent;
        oldIndex = sourceTransformIndex;

        //if (type == InsertionType::AFTER){
            // if position target has children, move them to the end of the list
            //targetTransformIndex = registry->findBranchLastIndex(target);
        //}
        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (needAdjustBranch){
            targetTransformIndex = targetTransformIndex - sizeOfSourceBranch;
        }

        moveEntityOrderByTransform(registry, entities, source, newParent, targetTransformIndex);

    }else{

        hasTransform = false;

        auto itSource = std::find(entities.begin(), entities.end(), source);
        auto itTarget = std::find(entities.begin(), entities.end(), target);

        if (itSource == entities.end() || itTarget == entities.end()) {
            Out::error("Source or Target entity not found in entities vector");
            return false;
        }

        oldIndex = std::distance(entities.begin(), itSource);

        Entity tempSource = *itSource;
        size_t sourceIndex = std::distance(entities.begin(), itSource);
        size_t targetIndex = std::distance(entities.begin(), itTarget);

        entities.erase(itSource);

        if (type == InsertionType::BEFORE) {
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex, tempSource);
        } else if (type == InsertionType::AFTER) {
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex + 1, tempSource);
        } else if (type == InsertionType::IN) {
            // "IN" is ambiguous without hierarchy, treat as AFTER
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex + 1, tempSource);
        }

    }

    return true;
}

void Editor::MoveEntityOrderCmd::moveEntityOrderByIndex(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t index, bool hasTransform){
    if (hasTransform){

        moveEntityOrderByTransform(registry, entities, source, parent, index);

    }else{

        auto itSource = std::find(entities.begin(), entities.end(), source);
        if (itSource == entities.end()) {
            Out::error("Source entity not found in entities vector for undo");
            return;
        }

        Entity tempSource = *itSource;
        entities.erase(itSource);

        // Clamp index for safety
        if (index > entities.size()) {
            entities.push_back(tempSource);
        } else {
            entities.insert(entities.begin() + index, tempSource);
        }

    }
}

void Editor::MoveEntityOrderCmd::moveEntityOrderByTransform(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t transformIndex, bool enableMove){
    registry->addEntityChild(parent, source, true);

    if (enableMove){
        registry->moveChildToIndex(source, transformIndex, false);
    }

    Project::sortEntitiesByTransformOrder(registry, entities);
}


bool Editor::MoveEntityOrderCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (project->isEntityShared(sceneId, source)){

        fs::path parentSharedPath;
        fs::path sourceSharedPath = project->findGroupPathFor(sceneId, source);

        if (type == InsertionType::IN){
            if (!project->isEntityShared(sceneId, target)){
                Out::error("Cannot move shared entity %u into target %u", source, target);
                return false;
            }
        }else{
            Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);
            if (transformTarget){
                parentSharedPath = project->findGroupPathFor(sceneId, transformTarget->parent);

                SharedGroup* sourceSourceGroup = project->getSharedGroup(sourceSharedPath);
                uint32_t souceInstanceId = sourceSourceGroup->getInstanceId(sceneId, source);

                bool isSourceRoot = sourceSourceGroup && (sourceSourceGroup->getRootEntity(sceneId, souceInstanceId) == source);

                if (parentSharedPath != sourceSharedPath && !isSourceRoot){
                    Out::error("Cannot move shared entity %u outside shared group", source);
                    return false;
                }
            }
        }

        if (parentSharedPath == sourceSharedPath){
            sharedMoveRecovery = project->moveEntityFromSharedGroup(sceneId, source, target, type, false);
        }
    }
    moveEntityOrderByTarget(sceneProject->scene, sceneProject->entities, source, target, type, oldParent, oldIndex, hasTransform);

    sceneProject->isModified = true;

    return true;
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (sharedMoveRecovery.size() > 0){
        project->undoMoveEntityInSharedGroup(sceneId, source, target, sharedMoveRecovery, false);
    }
    moveEntityOrderByIndex(sceneProject->scene, sceneProject->entities, source, oldParent, oldIndex, hasTransform);

    sceneProject->isModified = wasModified;
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}