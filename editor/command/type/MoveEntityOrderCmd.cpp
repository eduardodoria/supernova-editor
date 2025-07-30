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

void Editor::MoveEntityOrderCmd::sortEntitiesByTransformOrder(std::vector<Entity>& entities, Scene* scene) {
    auto transforms = scene->getComponentArray<Transform>();
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

bool Editor::MoveEntityOrderCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneProject->entities;

    Transform* transformSource = sceneProject->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){

        if (sceneProject->scene->isParentOf(source, target)){
            Out::error("Cannot move entity to a child");
            return false;
        }

        auto transforms = sceneProject->scene->getComponentArray<Transform>();

        size_t sourceTransformIndex = transforms->getIndex(source);
        size_t targetTransformIndex = transforms->getIndex(target);

        // Need to be before addEntityChild
        size_t sizeOfSourceBranch = sceneProject->scene->findBranchLastIndex(source) - sourceTransformIndex + 1;
        bool needAdjustBranch = (sourceTransformIndex < targetTransformIndex);

        Entity newParent = NULL_ENTITY;
        if (type == InsertionType::IN){
            newParent = target;
        }else{
            newParent = transformTarget->parent;
        }

        oldParent = transformSource->parent;
        sceneProject->scene->addEntityChild(newParent, source, true);

        oldTransformIndex = sourceTransformIndex;

        if (type == InsertionType::AFTER){
            // if position target has children, move them to the end of the list
            targetTransformIndex = sceneProject->scene->findBranchLastIndex(target);
        }
        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (needAdjustBranch){
            targetTransformIndex = targetTransformIndex - sizeOfSourceBranch;
        }

        sceneProject->scene->moveChildToIndex(source, targetTransformIndex, false);

        sortEntitiesByTransformOrder(entities, sceneProject->scene);

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

    sceneProject->isModified = true;

    return true;
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneProject->entities;

    Transform* transformSource = sceneProject->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){

        sceneProject->scene->addEntityChild(oldParent, source, true);

        sceneProject->scene->moveChildToIndex(source, oldTransformIndex, false);

        sortEntitiesByTransformOrder(entities, sceneProject->scene);

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

    sceneProject->isModified = wasModified;
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}