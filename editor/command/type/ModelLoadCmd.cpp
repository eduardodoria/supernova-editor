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

    Signature signature = sceneProject->scene->getSignature(entity);
    if (!signature.test(sceneProject->scene->getComponentId<Transform>())){
        Log::error("Entity %lu does not have a Transform component", entity);
        return false;
    }
    if (!signature.test(sceneProject->scene->getComponentId<MeshComponent>())){
        Log::error("Entity %lu does not have a MeshComponent", entity);
        return false;
    }
    if (!signature.test(sceneProject->scene->getComponentId<ModelComponent>())){
        Log::error("Entity %lu does not have a ModelComponent", entity);
        return false;
    }

    Transform& transform = sceneProject->scene->getComponent<Transform>(entity);
    MeshComponent& mesh = sceneProject->scene->getComponent<MeshComponent>(entity);
    ModelComponent& model = sceneProject->scene->getComponent<ModelComponent>(entity);

    //oldModel = Stream::encodeEntity(entity, sceneProject->scene);
    oldTransform = Stream::encodeTransform(transform);
    oldMesh = Stream::encodeMeshComponent(mesh, false);
    oldModel = Stream::encodeModelComponent(model);
    for (const auto& bone : model.bonesIdMapping){
        oldBones.push_back(Stream::encodeEntity(bone.second, sceneProject->scene));
    }
    for (Entity animation : model.animations){
        oldAnimations.push_back(Stream::encodeEntity(animation, sceneProject->scene));
    }

    // Collect old generated entities to remove from scene tracking
    oldAddedEntities.clear();
    collectModelEntities(sceneProject->scene, model, oldAddedEntities);
    removeEntitiesFromScene(sceneProject, oldAddedEntities);

    std::shared_ptr<MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
    meshSys->clearBoneMapping(model);
    meshSys->clearAnimationMapping(model);

    std::string ext = FileData::getFilePathExtension(modelPath);
    bool ret = false;
    if (ext == "obj"){
        ret = meshSys->loadOBJ(entity, modelPath, false);
    }else{
        ret = meshSys->loadGLTF(entity, modelPath, false);
    }

    if (!ret){
        return false;
    }

    // Collect new generated entities and add to scene tracking
    addedEntities.clear();
    collectModelEntities(sceneProject->scene, model, addedEntities);
    addEntitiesToScene(sceneProject, addedEntities);

    sceneProject->isModified = true;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::Transform, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }

    return true;
}

void Editor::ModelLoadCmd::undo(){
    SceneProject* sceneProject = project->getScene(sceneId);

    ModelComponent& model = sceneProject->scene->getComponent<ModelComponent>(entity);

    // Remove new model entities from scene tracking
    removeEntitiesFromScene(sceneProject, addedEntities);

    std::shared_ptr<MeshSystem> meshSys = sceneProject->scene->getSystem<MeshSystem>();
    meshSys->clearBoneMapping(model);
    meshSys->clearAnimationMapping(model);

    // Recreate old bone and animation entities from saved YAML
    for (const auto& boneNode : oldBones){
        Stream::decodeEntity(boneNode, sceneProject->scene);
    }
    for (const auto& animNode : oldAnimations){
        Stream::decodeEntity(animNode, sceneProject->scene);
    }

    sceneProject->scene->getComponent<Transform>(entity) = Stream::decodeTransform(oldTransform);
    sceneProject->scene->getComponent<MeshComponent>(entity) = Stream::decodeMeshComponent(oldMesh);
    sceneProject->scene->getComponent<ModelComponent>(entity) = Stream::decodeModelComponent(oldModel);

    // Add old model entities back to scene tracking
    addEntitiesToScene(sceneProject, oldAddedEntities);

    sceneProject->isModified = wasModified;

    if (project->isEntityInBundle(sceneId, entity)){
        project->bundlePropertyChanged(sceneId, entity, ComponentType::Transform, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::MeshComponent, {});
        project->bundlePropertyChanged(sceneId, entity, ComponentType::ModelComponent, {});
    }
}

bool Editor::ModelLoadCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}
