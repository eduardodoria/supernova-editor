#include "MoveEntityOrderCmd.h"

using namespace Supernova;        
        
Editor::MoveEntityOrderCmd::MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type){
    this->project = project;
    this->sceneId = sceneId;
    this->source = source;
    this->target = target;
    this->type = type;
}

size_t Editor::MoveEntityOrderCmd::getIndex(std::vector<Entity>& entities, Entity entity){
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
        return std::distance(entities.begin(), it);
    }

    throw std::out_of_range("cannot find entity");
}

void Editor::MoveEntityOrderCmd::execute(){
    SceneData* sceneData = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneData->entities;

    Transform* transformSource = sceneData->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneData->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        auto transforms = sceneData->scene->getComponentArray<Transform>();

        size_t sourceTransformIndex = transforms->getIndex(source);
        size_t targetTransformIndex = transforms->getIndex(target);

        Entity newParent = NULL_ENTITY;
        if (type == InsertionType::IN){
            newParent = target;
        }else{
            newParent = transformTarget->parent;
        }

        oldParent = transformSource->parent;
        sceneData->scene->addEntityChild(newParent, source, true);

        oldTransformIndex = sourceTransformIndex;

        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (sourceTransformIndex < targetTransformIndex){
            --targetTransformIndex;
        }

        sceneData->scene->moveChildToIndex(source, targetTransformIndex);
    }

    size_t sourceIndex = getIndex(entities, source);
    size_t targetIndex = getIndex(entities, target);

    oldIndex = sourceIndex;

    if (type == InsertionType::AFTER || type == InsertionType::IN){
        targetIndex++;
    }
    if (sourceIndex < targetIndex){
        --targetIndex;
    }

    entities.erase(entities.begin() + sourceIndex);
    entities.insert(entities.begin() + targetIndex, source);
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneData* sceneData = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneData->entities;

    Transform* transformSource = sceneData->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneData->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        sceneData->scene->addEntityChild(oldParent, source, true);

        sceneData->scene->moveChildToIndex(source, oldTransformIndex);
    }

    size_t sourceIndex = getIndex(entities, source);

    entities.erase(entities.begin() + sourceIndex);
    entities.insert(entities.begin() + oldIndex, source);
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}