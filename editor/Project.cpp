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

std::vector<Editor::SceneData>&  Editor::Project::getScenes(){
    return scenes;
}

Editor::SceneData* Editor::Project::getScene(uint32_t sceneId){
    for (int i = 0; i < scenes.size(); i++){
        if (scenes[i].id == sceneId){
            return &scenes[i];
        }
    }

    throw std::out_of_range("cannot find selected scene");
}

Editor::SceneData* Editor::Project::getSelectedScene(){
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