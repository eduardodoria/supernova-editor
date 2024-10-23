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

    size_t sourceIndex = getIndex(entities, source);
    size_t targetIndex = getIndex(entities, target);

    oldIndex = sourceIndex;
    if (Transform* transform = sceneData->scene->findComponent<Transform>(source)){
        oldParent = transform->parent;
    }

    if (type == InsertionType::IN){
        sceneData->scene->addEntityChild(target, source, true);
    }

    if (type == InsertionType::AFTER){
        targetIndex++;
    }

    entities.erase(entities.begin() + sourceIndex);

    if (sourceIndex < targetIndex){
        --targetIndex;
    }

    entities.insert(entities.begin() + targetIndex, source);
}

void Editor::MoveEntityOrderCmd::undo(){
    SceneData* sceneData = project->getScene(sceneId);
    std::vector<Entity>& entities = sceneData->entities;

    if (type == InsertionType::IN){
        sceneData->scene->addEntityChild(oldParent, source, true);
    }

    size_t sourceIndex = getIndex(entities, source);

    entities.erase(entities.begin() + sourceIndex);

    entities.insert(entities.begin() + oldIndex, source);
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}