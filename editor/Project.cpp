#include "Project.h"

#include "Backend.h"

#include "subsystem/MeshSystem.h"
#include "command/type/CreateEntityCmd.h"
#include "command/type/DeleteEntityCmd.h"

using namespace Supernova;

uint32_t Editor::Project::nextSceneId = 0;

Editor::Project::Project(){
    selectedScene = NULL_PROJECT_SCENE;
    lastActivatedScene = NULL_PROJECT_SCENE;
}

uint32_t Editor::Project::createNewScene(std::string sceneName){
    unsigned int nameCount = 2;
    std::string baseName = sceneName;
    bool foundName = true;
    while (foundName){
        foundName = false;
        for (auto& sceneProject : scenes) {
            std::string usedName = sceneProject.name;
            if (usedName == sceneName){
                sceneName = baseName + " " + std::to_string(nameCount);
                nameCount++;
                foundName = true;
            }
        }
    }

    SceneProject data;
    data.id = ++nextSceneId;
    data.name = sceneName;
    data.scene = new Scene();
    data.sceneRender = new SceneRender(data.scene);
    data.selectedEntities.clear();
    data.needUpdateRender = true;

    scenes.push_back(data);

    setSelectedSceneId(scenes.back().id);

    Backend::getApp().addNewSceneToDock(scenes.back().id);

    return scenes.back().id;
}

Entity Editor::Project::createNewEntity(uint32_t sceneId, std::string entityName){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            Entity entity = scenes[i].scene->createEntity();
            scenes[i].scene->setEntityName(entity, entityName);

            scenes[i].entities.push_back(entity);

            setSelectedEntity(sceneId, entity);

            return entity;
        }
    }

    return NULL_ENTITY;
}

bool Editor::Project::createNewComponent(uint32_t sceneId, Entity entity, ComponentType component){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            if (component == ComponentType::Transform){
                scenes[i].scene->addComponent<Transform>(entity, {});
            }
            if (component == ComponentType::MeshComponent){
                scenes[i].scene->addComponent<MeshComponent>(entity, {});
            }
            return true;
        }
    }

    return false;
}

void Editor::Project::createEmptyEntity(uint32_t sceneId){
    CommandHistory::addCommand(new CreateEntityCmd(this, sceneId, "Entity"));
}

void Editor::Project::createBoxShape(uint32_t sceneId, Entity parent){
    CommandHistory::addCommand(new CreateEntityCmd(this, sceneId, "Box", EntityCreationType::BOX, parent));
}

void Editor::Project::deleteEntity(uint32_t sceneId, Entity entity){
    CommandHistory::addCommand(new DeleteEntityCmd(this, sceneId, entity));
}

bool Editor::Project::findObjectByRay(uint32_t sceneId, float x, float y){
    SceneProject* scenedata = getScene(sceneId);
    Ray ray = scenedata->sceneRender->getCamera()->screenToRay(x, y);

    float distance = FLT_MAX;
    Entity selEntity = NULL_ENTITY;
    for (auto& entity : scenedata->entities) {
        MeshComponent* mesh = scenedata->scene->findComponent<MeshComponent>(entity);
        if (mesh){
            RayReturn rreturn = ray.intersects(mesh->worldAABB);
            if (rreturn.hit){
                if (rreturn.distance < distance){
                    distance = rreturn.distance;
                    selEntity = entity;
                }
            }
        }
    }

    if (!scenedata->sceneRender->isGizmoSideSelected()){
        if (selEntity != NULL_ENTITY){
            setSelectedEntity(sceneId, selEntity);
            return true;
        }

        clearSelectedEntities(sceneId);
    }

    return false;
}

bool Editor::Project::findObjectsByRect(uint32_t sceneId, Vector2 start, Vector2 end){
    SceneProject* scenedata = getScene(sceneId);
    Camera* camera = scenedata->sceneRender->getCamera();

    clearSelectedEntities(sceneId);

    float distance = FLT_MAX;
    Entity selEntity = NULL_ENTITY;
    for (auto& entity : scenedata->entities) {
        Transform* transform = scenedata->scene->findComponent<Transform>(entity);
        MeshComponent* mesh = scenedata->scene->findComponent<MeshComponent>(entity);
        if (transform && mesh){
            const Vector3* corners = mesh->aabb.getAllCorners();

            Vector2 minRect = Vector2(std::min(start.x, end.x), std::min(start.y, end.y));
            Vector2 maxRect = Vector2(std::max(start.x, end.x), std::max(start.y, end.y));

            bool found = true;

            for (int c = 0; c < 8; c++){
                Vector4 clipCorner = camera->getViewProjectionMatrix() * transform->modelMatrix * Vector4(corners[c], 1.0);
                Vector3 ndcCorner = Vector3(clipCorner) / clipCorner.w;

                if (!(ndcCorner.x >= minRect.x && ndcCorner.x <= maxRect.x && ndcCorner.y >= minRect.y && ndcCorner.y <= maxRect.y)){
                    found = false;
                    break;
                }
            }

            if (found){
                //printf("Found entity %u\n", entity);
                addSelectedEntity(sceneId, entity);
            }
        }
    }

    return false;
}

std::vector<Editor::SceneProject>&  Editor::Project::getScenes(){
    return scenes;
}

template<typename T>
T* Editor::Project::findScene(uint32_t sceneId) const {
    for (int i = 0; i < scenes.size(); i++) {
        if (scenes[i].id == sceneId) {
            return const_cast<T*>(&scenes[i]);
        }
    }
    throw std::out_of_range("cannot find selected scene");
}


// Non-const version
Editor::SceneProject* Editor::Project::getScene(uint32_t sceneId) {
    return findScene<Editor::SceneProject>(sceneId);
}

// Const version
const Editor::SceneProject* Editor::Project::getScene(uint32_t sceneId) const {
    return findScene<const Editor::SceneProject>(sceneId);
}

Editor::SceneProject* Editor::Project::getSelectedScene(){
    return getScene(selectedScene);
}

const Editor::SceneProject* Editor::Project::getSelectedScene() const{
    return getScene(selectedScene);
}

void Editor::Project::setSelectedSceneId(uint32_t selectedScene){
    if (this->selectedScene != selectedScene){
        this->selectedScene = selectedScene;
    }
}

uint32_t Editor::Project::getSelectedSceneId() const{
    return selectedScene;
}

void Editor::Project::setLastActivatedSceneId(uint32_t lastActivatedScene){
    this->lastActivatedScene = lastActivatedScene;
}

uint32_t Editor::Project::getLastActivatedSceneId() const{
    return lastActivatedScene;
}

void Editor::Project::replaceSelectedEntities(uint32_t sceneId, std::vector<Entity> selectedEntities){
    getScene(sceneId)->selectedEntities = selectedEntities;
}

void Editor::Project::setSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;

    entities.clear();
    if (selectedEntity != NULL_ENTITY){
        entities.push_back(selectedEntity);
    }
}

void Editor::Project::addSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;

    if (selectedEntity != NULL_ENTITY){
        if (std::find(entities.begin(), entities.end(), selectedEntity) == entities.end()) {
            entities.push_back(selectedEntity);
        }
    }
}

bool Editor::Project::isSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    std::vector<Entity>& entities = getScene(sceneId)->selectedEntities;

    if (std::find(entities.begin(), entities.end(), selectedEntity) != entities.end()) {
        return true;
    }

    return false;
}

void Editor::Project::clearSelectedEntities(uint32_t sceneId){
    getScene(sceneId)->selectedEntities.clear();
}

std::vector<Entity> Editor::Project::getSelectedEntities(uint32_t sceneId) const{
    return getScene(sceneId)->selectedEntities;
}

bool Editor::Project::hasSelectedEntities(uint32_t sceneId) const{
    return (getScene(sceneId)->selectedEntities.size() > 0);
}