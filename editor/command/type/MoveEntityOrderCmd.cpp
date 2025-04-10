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
    SceneProject* sceneProject = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneProject->entities;

    Transform* transformSource = sceneProject->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        auto transforms = sceneProject->scene->getComponentArray<Transform>();

        size_t sourceTransformIndex = transforms->getIndex(source);
        size_t targetTransformIndex = transforms->getIndex(target);

        Entity newParent = NULL_ENTITY;
        if (type == InsertionType::IN){
            newParent = target;
        }else{
            newParent = transformTarget->parent;
        }

        oldParent = transformSource->parent;
        sceneProject->scene->addEntityChild(newParent, source, true);

        oldTransformIndex = sourceTransformIndex;

        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (sourceTransformIndex < targetTransformIndex){
            --targetTransformIndex;
        }

        sceneProject->scene->moveChildToIndex(source, targetTransformIndex, false);
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
    SceneProject* sceneProject = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneProject->entities;

    Transform* transformSource = sceneProject->scene->findComponent<Transform>(source);
    Transform* transformTarget = sceneProject->scene->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        sceneProject->scene->addEntityChild(oldParent, source, true);

        sceneProject->scene->moveChildToIndex(source, oldTransformIndex, false);
    }

    size_t sourceIndex = getIndex(entities, source);

    entities.erase(entities.begin() + sourceIndex);
    entities.insert(entities.begin() + oldIndex, source);
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}