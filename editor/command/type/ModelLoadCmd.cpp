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

void Editor::ModelLoadCmd::collectModelEntities(Scene* scene, const ModelComponent& model, std::vector<Entity>& out){
    for (const auto& bone : model.bonesIdMapping){
        out.push_back(bone.second);
    }
    for (const auto& anim : model.animations){
        out.push_back(anim);
        AnimationComponent* animComp = scene->findComponent<AnimationComponent>(anim);
        if (animComp){
            for (const auto& frame : animComp->actions){
                if (frame.action != NULL_ENTITY){
                    out.push_back(frame.action);
                }
            }
        }
    }
}

void Editor::ModelLoadCmd::addEntitiesToScene(SceneProject* sceneProject, const std::vector<Entity>& ents){
    for (const auto& e : ents){
        sceneProject->entities.push_back(e);
    }
}

void Editor::ModelLoadCmd::removeEntitiesFromScene(SceneProject* sceneProject, const std::vector<Entity>& ents){
    for (const auto& e : ents){
        auto it = std::find(sceneProject->entities.begin(), sceneProject->entities.end(), e);
        if (it != sceneProject->entities.end()){
            sceneProject->entities.erase(it);
        }
    }
}

bool Editor::ModelLoadCmd::execute(){
    SceneProject* sceneProject = project->getScene(sceneId);

    oldMesh = Stream::encodeMeshComponent(sceneProject->scene->getComponent<MeshComponent>(entity));
    oldModel = Stream::encodeModelComponent(sceneProject->scene->getComponent<ModelComponent>(entity));
    oldModelPath = sceneProject->scene->getComponent<ModelComponent>(entity).filename;

    // Collect old generated entities to remove from scene tracking
    oldAddedEntities.clear();
    collectModelEntities(sceneProject->scene, sceneProject->scene->getComponent<ModelComponent>(entity), oldAddedEntities);
    removeEntitiesFromScene(sceneProject, oldAddedEntities);

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

    // Collect new generated entities and add to scene tracking
    addedEntities.clear();
    collectModelEntities(sceneProject->scene, sceneProject->scene->getComponent<ModelComponent>(entity), addedEntities);
    addEntitiesToScene(sceneProject, addedEntities);

    sceneProject->isModified = true;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }

    return true;
}

void Editor::ModelLoadCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    // Remove current generated entities from scene tracking
    removeEntitiesFromScene(sceneProject, addedEntities);

    // Destroy the currently loaded generated child entities before restoring the old model state
    std::shared_ptr<MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
    meshSys->destroyModel(sceneProject->scene->getComponent<ModelComponent>(entity));

    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);
    sceneProject->scene->getComponent<ModelComponent>(entity) = Stream::decodeModelComponent(oldModel);
    sceneProject->scene->getComponent<ModelComponent>(entity).filename = oldModelPath;
    sceneProject->scene->getComponent<ModelComponent>(entity).needUpdateModel = true;
    sceneProject->scene->getComponent<MeshComponent>(entity).needReload = true;

    // Restore old generated entities to scene tracking
    addEntitiesToScene(sceneProject, oldAddedEntities);

    sceneProject->isModified = wasModified;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }
}

bool Editor::ModelLoadCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}
