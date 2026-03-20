#include "ModelLoadCmd.h"

#include "Stream.h"
#include "subsystem/MeshSystem.h"
#include "io/FileData.h"
#include "component/AnimationComponent.h"

using namespace Supernova;

Editor::ModelLoadCmd::ModelLoadCmd(Project* project, uint32_t sceneId, Entity entity, const std::string& modelPath){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->modelPath = modelPath;

    this->wasModified = project->getScene(sceneId)->isModified;
}

bool Editor::ModelLoadCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    oldMesh = Stream::encodeMeshComponent(sceneProject->scene->getComponent<MeshComponent>(entity));
    oldModel = Stream::encodeModelComponent(sceneProject->scene->getComponent<ModelComponent>(entity));
    oldModelPath = sceneProject->scene->getComponent<ModelComponent>(entity).filename;

    // Load model from file (MeshSystem::destroyModel handles cleanup of old entities)
    std::shared_ptr<MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();

    std::string ext = FileData::getFilePathExtension(modelPath);
    bool ret = false;
    if (ext == "obj"){
        ret = meshSys->loadOBJ(entity, modelPath);
    }else{
        ret = meshSys->loadGLTF(entity, modelPath);
    }

    if (!ret){
        return false;
    }

    sceneProject->scene->getComponent<MeshComponent>(entity).needReload = true;

    sceneProject->scene->getComponent<ModelComponent>(entity).filename = modelPath;

    sceneProject->isModified = true;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }

    return true;
}

void Editor::ModelLoadCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    // Destroy the currently loaded generated child entities before restoring the old model state
    std::shared_ptr<MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
    meshSys->destroyModel(sceneProject->scene->getComponent<ModelComponent>(entity));

    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);
    sceneProject->scene->getComponent<ModelComponent>(entity) = Stream::decodeModelComponent(oldModel);
    sceneProject->scene->getComponent<ModelComponent>(entity).filename = oldModelPath;
    sceneProject->scene->getComponent<ModelComponent>(entity).needUpdateModel = true;
    sceneProject->scene->getComponent<MeshComponent>(entity).needReload = true;

    sceneProject->isModified = wasModified;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }
}

bool Editor::ModelLoadCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}
