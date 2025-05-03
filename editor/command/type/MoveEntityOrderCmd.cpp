#include "MoveEntityOrderCmd.h"

#include "Out.h"

using namespace Supernova;        
        
Editor::MoveEntityOrderCmd::MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type){
    this->project = project;
    this->sceneId = sceneId;
    this->source = source;
    this->target = target;
    this->type = type;
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
        if (sourceTransformIndex < targetTransformIndex){
            --targetTransformIndex;
        }

        sceneProject->scene->moveChildToIndex(source, targetTransformIndex, false);
    }

    sortEntitiesByTransformOrder(entities, sceneProject->scene);

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
    }

    sortEntitiesByTransformOrder(entities, sceneProject->scene);

    sceneProject->isModified = true;
}

bool Editor::MoveEntityOrderCmd::mergeWith(Command* otherCommand){
    return false;
}