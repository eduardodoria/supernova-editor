#include "Project.h"

#include "subsystem/MeshSystem.h"

using namespace Supernova;

uint32_t Editor::Project::nextSceneId = 0;

Editor::Project::Project(){
    selectedScene = NULL_PROJECT_SCENE;
}

uint32_t Editor::Project::createNewScene(std::string sceneName){
    SceneData data;
    data.id = ++nextSceneId;
    data.name = sceneName;
    data.scene = new Scene();
    data.sceneRender = new SceneRender(data.scene);
    data.selectedEntity = NULL_ENTITY;
    data.needUpdateRender = true;

    scenes.push_back(data);

    setSelectedSceneId(scenes.back().id);

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

void Editor::Project::createBoxShape(uint32_t sceneId){
    Entity box = createNewEntity(sceneId, "Box");
    createNewComponent(sceneId, box, ComponentType::Transform);
    createNewComponent(sceneId, box, ComponentType::MeshComponent);

    Scene* scene = getScene(sceneId)->scene;
    MeshComponent& mesh = scene->getComponent<MeshComponent>(box);

    scene->getSystem<MeshSystem>()->createBox(box, 1, 1, 1);
    //mesh.submeshes[0].material.baseColorFactor = Color::sRGBToLinear(Vector4(0.5, 0.5, 0.5, 1.0));
}

bool Editor::Project::findObjectByRay(uint32_t sceneId, float x, float y){
    SceneData* scenedata = getScene(sceneId);
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

    if (selEntity != NULL_ENTITY){
        setSelectedEntity(sceneId, selEntity);
        return true;
    }

    if (!scenedata->sceneRender->isGizmoSideSelected()){
        setSelectedEntity(sceneId, NULL_ENTITY);
    }

    return false;
}

std::vector<Editor::SceneData>&  Editor::Project::getScenes(){
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
Editor::SceneData* Editor::Project::getScene(uint32_t sceneId) {
    return findScene<Editor::SceneData>(sceneId);
}

// Const version
const Editor::SceneData* Editor::Project::getScene(uint32_t sceneId) const {
    return findScene<const Editor::SceneData>(sceneId);
}

Editor::SceneData* Editor::Project::getSelectedScene(){
    return getScene(selectedScene);
}

const Editor::SceneData* Editor::Project::getSelectedScene() const{
    return getScene(selectedScene);
}

void Editor::Project::setSelectedSceneId(uint32_t selectedScene){
    if (this->selectedScene != selectedScene){
        this->selectedScene = selectedScene;
        getScene(selectedScene)->sceneRender->activate();
    }
}

uint32_t Editor::Project::getSelectedSceneId() const{
    return selectedScene;
}

void Editor::Project::setSelectedEntity(uint32_t sceneId, Entity selectedEntity){
    SceneData* sceneData = getScene(sceneId);
    if (sceneData->selectedEntity != selectedEntity){
        sceneData->selectedEntity = selectedEntity;
    }
}

Entity Editor::Project::getSelectedEntity(uint32_t sceneId) const{
    return getScene(sceneId)->selectedEntity;
}