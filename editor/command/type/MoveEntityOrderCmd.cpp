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

void Editor::MoveEntityOrderCmd::sortEntitiesByTransformOrder(std::vector<Entity>& entities, EntityRegistry* registry) {
    auto transforms = registry->getComponentArray<Transform>();
    std::unordered_map<Entity, size_t> transformOrder;
    for (size_t i = 0; i < transforms->size(); ++i) {
        Entity ent = transforms->getEntity(i);
        transformOrder[ent] = i;
    }
    std::sort(entities.begin(), entities.end(),
        [&transformOrder](const Entity& a, const Entity& b) {
            return transformOrder[a] < transformOrder[b];
        }
    );
}

bool Editor::MoveEntityOrderCmd::changeEntityOrder(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, size_t& oldTransformIndex) {
    Transform* transformSource = registry->findComponent<Transform>(source);
    Transform* transformTarget = registry->findComponent<Transform>(target);

    if (transformSource && transformTarget){

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
        registry->addEntityChild(newParent, source, true);

        oldTransformIndex = sourceTransformIndex;

        if (type == InsertionType::AFTER){
            // if position target has children, move them to the end of the list
            targetTransformIndex = registry->findBranchLastIndex(target);
        }
        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (needAdjustBranch){
            targetTransformIndex = targetTransformIndex - sizeOfSourceBranch;
        }

        registry->moveChildToIndex(source, targetTransformIndex, false);

        sortEntitiesByTransformOrder(entities, registry);

    }else{

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

void Editor::MoveEntityOrderCmd::undoEntityOrder(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, Entity oldParent, size_t oldIndex, size_t oldTransformIndex){
    Transform* transformSource = registry->findComponent<Transform>(source);
    Transform* transformTarget = registry->findComponent<Transform>(target);

    if (transformSource && transformTarget){

        registry->addEntityChild(oldParent, source, true);

        registry->moveChildToIndex(source, oldTransformIndex, false);

        sortEntitiesByTransformOrder(entities, registry);

    }else{

        auto itSource = std::find(entities.begin(), entities.end(), source);
        if (itSource == entities.end()) {
            Out::error("Source entity not found in entities vector for undo");
            return;
        }

        Entity tempSource = *itSource;
        entities.erase(itSource);

        // Clamp oldIndex for safety
        if (oldIndex > entities.size()) {
            entities.push_back(tempSource);
        } else {
            entities.insert(entities.begin() + oldIndex, tempSource);
        }

    }
}


bool Editor::MoveEntityOrderCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (project->isEntityShared(sceneId, source)){
        sharedMoveRecovery = project->moveEntityFromSharedGroup(sceneId, source, target, type, false);
    }
    changeEntityOrder(sceneProject->scene, sceneProject->entities, source, target, type, oldParent, oldIndex, oldTransformIndex);

    sceneProject->isModified = true;

    return true;
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    if (sharedMoveRecovery.size() > 0){
        project->undoMoveEntityInSharedGroup(sceneId, source, target, sharedMoveRecovery, false);
    }
    undoEntityOrder(sceneProject->scene, sceneProject->entities, source, target, oldParent, oldIndex, oldTransformIndex);

    sceneProject->isModified = wasModified;
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}