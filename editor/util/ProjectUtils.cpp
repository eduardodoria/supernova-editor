#include "ProjectUtils.h"

#include "Out.h"
#include "Catalog.h"
#include "Stream.h"

using namespace Supernova;

size_t Editor::ProjectUtils::getTransformIndex(EntityRegistry* registry, Entity entity){
    Signature signature = registry->getSignature(entity);
    if (signature.test(registry->getComponentId<Transform>())) {
        Transform& transform = registry->getComponent<Transform>(entity);
        auto transforms = registry->getComponentArray<Transform>();
        return transforms->getIndex(entity);
    }

    return 0;
}

void Editor::ProjectUtils::sortEntitiesByTransformOrder(EntityRegistry* registry, std::vector<Entity>& entities) {
    auto transforms = registry->getComponentArray<Transform>();
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

bool Editor::ProjectUtils::moveEntityOrderByTarget(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, bool& hasTransform) {
    Transform* transformSource = registry->findComponent<Transform>(source);
    Transform* transformTarget = registry->findComponent<Transform>(target);

    if (transformSource && transformTarget){
        hasTransform = true;

        if (registry->isParentOf(source, target)){
            Out::error("Cannot move entity to a child");
            return false;
        }

        auto transforms = registry->getComponentArray<Transform>();

        size_t sourceTransformIndex = transforms->getIndex(source);
        size_t targetTransformIndex = transforms->getIndex(target);

        // Need to be before addEntityChild
        size_t sizeOfSourceBranch = registry->findBranchLastIndex(source) - sourceTransformIndex + 1;
        bool needAdjustBranch = (sourceTransformIndex < targetTransformIndex);

        Entity newParent = NULL_ENTITY;
        if (type == InsertionType::IN){
            newParent = target;
        }else{
            newParent = transformTarget->parent;
        }

        oldParent = transformSource->parent;
        oldIndex = sourceTransformIndex;

        //if (type == InsertionType::AFTER){
            // if position target has children, move them to the end of the list
            //targetTransformIndex = registry->findBranchLastIndex(target);
        //}
        if (type == InsertionType::AFTER || type == InsertionType::IN){
            targetTransformIndex++;
        }
        if (needAdjustBranch){
            targetTransformIndex = targetTransformIndex - sizeOfSourceBranch;
        }

        moveEntityOrderByTransform(registry, entities, source, newParent, targetTransformIndex);

    }else{

        hasTransform = false;

        auto itSource = std::find(entities.begin(), entities.end(), source);
        auto itTarget = std::find(entities.begin(), entities.end(), target);

        if (itSource == entities.end() || itTarget == entities.end()) {
            Out::error("Source or Target entity not found in entities vector");
            return false;
        }

        oldIndex = std::distance(entities.begin(), itSource);

        Entity tempSource = *itSource;
        size_t sourceIndex = std::distance(entities.begin(), itSource);
        size_t targetIndex = std::distance(entities.begin(), itTarget);

        entities.erase(itSource);

        if (type == InsertionType::BEFORE) {
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex, tempSource);
        } else if (type == InsertionType::AFTER) {
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex + 1, tempSource);
        } else if (type == InsertionType::IN) {
            // "IN" is ambiguous without hierarchy, treat as AFTER
            if (sourceIndex < targetIndex) targetIndex--;
            entities.insert(entities.begin() + targetIndex + 1, tempSource);
        }

    }

    return true;
}

void Editor::ProjectUtils::moveEntityOrderByIndex(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t index, bool hasTransform){
    if (hasTransform){

        moveEntityOrderByTransform(registry, entities, source, parent, index);

    }else{

        auto itSource = std::find(entities.begin(), entities.end(), source);
        if (itSource == entities.end()) {
            Out::error("Source entity not found in entities vector for undo");
            return;
        }

        Entity tempSource = *itSource;
        entities.erase(itSource);

        // Clamp index for safety
        if (index > entities.size()) {
            entities.push_back(tempSource);
        } else {
            entities.insert(entities.begin() + index, tempSource);
        }

    }
}

void Editor::ProjectUtils::moveEntityOrderByTransform(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t transformIndex, bool enableMove){
    registry->addEntityChild(parent, source, true);

    if (enableMove){
        registry->moveChildToIndex(source, transformIndex, false);
    }

    ProjectUtils::sortEntitiesByTransformOrder(registry, entities);
}

void Editor::ProjectUtils::addEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, YAML::Node componentNode){
    switch (componentType) {
        case ComponentType::Transform:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<Transform>(entity, {});
            }else{
                registry->addComponent<Transform>(entity, Stream::decodeTransform(componentNode));
            }
            ProjectUtils::sortEntitiesByTransformOrder(registry, entities);
            break;
        case ComponentType::MeshComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<MeshComponent>(entity, {});
            }else{
                registry->addComponent<MeshComponent>(entity, Stream::decodeMeshComponent(componentNode));
            }
            break;
        case ComponentType::UIComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<UIComponent>(entity, {});
            }else{
                registry->addComponent<UIComponent>(entity, Stream::decodeUIComponent(componentNode));
            }
            break;
        case ComponentType::UILayoutComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<UILayoutComponent>(entity, {});
            }else{
                registry->addComponent<UILayoutComponent>(entity, Stream::decodeUILayoutComponent(componentNode));
            }
            break;
        case ComponentType::ActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ActionComponent>(entity, {});
            }else{
                registry->addComponent<ActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::AlphaActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<AlphaActionComponent>(entity, {});
            }else{
                registry->addComponent<AlphaActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::AnimationComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<AnimationComponent>(entity, {});
            }else{
                registry->addComponent<AnimationComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::AudioComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<AudioComponent>(entity, {});
            }else{
                registry->addComponent<AudioComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::Body2DComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<Body2DComponent>(entity, {});
            }else{
                registry->addComponent<Body2DComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::Body3DComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<Body3DComponent>(entity, {});
            }else{
                registry->addComponent<Body3DComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::BoneComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<BoneComponent>(entity, {});
            }else{
                registry->addComponent<BoneComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ButtonComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ButtonComponent>(entity, {});
            }else{
                registry->addComponent<ButtonComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::CameraComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<CameraComponent>(entity, {});
            }else{
                registry->addComponent<CameraComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ColorActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ColorActionComponent>(entity, {});
            }else{
                registry->addComponent<ColorActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::FogComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<FogComponent>(entity, {});
            }else{
                registry->addComponent<FogComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ImageComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ImageComponent>(entity, {});
            }else{
                registry->addComponent<ImageComponent>(entity, Stream::decodeImageComponent(componentNode));
            }
            break;
        case ComponentType::InstancedMeshComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<InstancedMeshComponent>(entity, {});
            }else{
                registry->addComponent<InstancedMeshComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::Joint2DComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<Joint2DComponent>(entity, {});
            }else{
                registry->addComponent<Joint2DComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::Joint3DComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<Joint3DComponent>(entity, {});
            }else{
                registry->addComponent<Joint3DComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::KeyframeTracksComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<KeyframeTracksComponent>(entity, {});
            }else{
                registry->addComponent<KeyframeTracksComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::LightComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<LightComponent>(entity, {});
            }else{
                registry->addComponent<LightComponent>(entity, Stream::decodeLightComponent(componentNode));
            }
            break;
        case ComponentType::LinesComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<LinesComponent>(entity, {});
            }else{
                registry->addComponent<LinesComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::MeshPolygonComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<MeshPolygonComponent>(entity, {});
            }else{
                registry->addComponent<MeshPolygonComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ModelComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ModelComponent>(entity, {});
            }else{
                registry->addComponent<ModelComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::MorphTracksComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<MorphTracksComponent>(entity, {});
            }else{
                registry->addComponent<MorphTracksComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::PanelComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<PanelComponent>(entity, {});
            }else{
                registry->addComponent<PanelComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ParticlesComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ParticlesComponent>(entity, {});
            }else{
                registry->addComponent<ParticlesComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::PointsComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<PointsComponent>(entity, {});
            }else{
                registry->addComponent<PointsComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::PolygonComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<PolygonComponent>(entity, {});
            }else{
                registry->addComponent<PolygonComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::PositionActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<PositionActionComponent>(entity, {});
            }else{
                registry->addComponent<PositionActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::RotateTracksComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<RotateTracksComponent>(entity, {});
            }else{
                registry->addComponent<RotateTracksComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::RotationActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<RotationActionComponent>(entity, {});
            }else{
                registry->addComponent<RotationActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ScaleActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ScaleActionComponent>(entity, {});
            }else{
                registry->addComponent<ScaleActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ScaleTracksComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ScaleTracksComponent>(entity, {});
            }else{
                registry->addComponent<ScaleTracksComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ScriptComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ScriptComponent>(entity, {});
            }else{
                registry->addComponent<ScriptComponent>(entity, Stream::decodeScriptComponent(componentNode));
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::ScrollbarComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<ScrollbarComponent>(entity, {});
            }else{
                registry->addComponent<ScrollbarComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::SkyComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<SkyComponent>(entity, {});
            }else{
                registry->addComponent<SkyComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::SpriteAnimationComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<SpriteAnimationComponent>(entity, {});
            }else{
                registry->addComponent<SpriteAnimationComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::SpriteComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<SpriteComponent>(entity, {});
            }else{
                registry->addComponent<SpriteComponent>(entity, Stream::decodeSpriteComponent(componentNode));
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TerrainComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TerrainComponent>(entity, {});
            }else{
                registry->addComponent<TerrainComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TextComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TextComponent>(entity, {});
            }else{
                registry->addComponent<TextComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TextEditComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TextEditComponent>(entity, {});
            }else{
                registry->addComponent<TextEditComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TilemapComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TilemapComponent>(entity, {});
            }else{
                registry->addComponent<TilemapComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TimedActionComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TimedActionComponent>(entity, {});
            }else{
                registry->addComponent<TimedActionComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::TranslateTracksComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<TranslateTracksComponent>(entity, {});
            }else{
                registry->addComponent<TranslateTracksComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        case ComponentType::UIContainerComponent:
            if (!componentNode.IsDefined() || componentNode.IsNull()){
                registry->addComponent<UIContainerComponent>(entity, {});
            }else{
                registry->addComponent<UIContainerComponent>(entity, {});
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            break;
        default:
            break;
    }
}

YAML::Node Editor::ProjectUtils::removeEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, bool encodeComponent){
    YAML::Node oldComponent;

    switch (componentType) {
        case ComponentType::Transform:
            if (encodeComponent){
                oldComponent = Stream::encodeTransform(registry->getComponent<Transform>(entity));
            }
            registry->removeComponent<Transform>(entity);
            ProjectUtils::sortEntitiesByTransformOrder(registry, entities);
            break;
        case ComponentType::MeshComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeMeshComponent(registry->getComponent<MeshComponent>(entity));
            }
            registry->removeComponent<MeshComponent>(entity);
            break;
        case ComponentType::UIComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeUIComponent(registry->getComponent<UIComponent>(entity));
            }
            registry->removeComponent<UIComponent>(entity);
            break;
        case ComponentType::UILayoutComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeUILayoutComponent(registry->getComponent<UILayoutComponent>(entity));
            }
            registry->removeComponent<UILayoutComponent>(entity);
            break;
        case ComponentType::ActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ActionComponent>(entity);
            break;
        case ComponentType::AlphaActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<AlphaActionComponent>(entity);
            break;
        case ComponentType::AnimationComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<AnimationComponent>(entity);
            break;
        case ComponentType::AudioComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<AudioComponent>(entity);
            break;
        case ComponentType::Body2DComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<Body2DComponent>(entity);
            break;
        case ComponentType::Body3DComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<Body3DComponent>(entity);
            break;
        case ComponentType::BoneComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<BoneComponent>(entity);
            break;
        case ComponentType::ButtonComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ButtonComponent>(entity);
            break;
        case ComponentType::CameraComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<CameraComponent>(entity);
            break;
        case ComponentType::ColorActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ColorActionComponent>(entity);
            break;
        case ComponentType::FogComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<FogComponent>(entity);
            break;
        case ComponentType::ImageComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeImageComponent(registry->getComponent<ImageComponent>(entity));
            }
            registry->removeComponent<ImageComponent>(entity);
            break;
        case ComponentType::InstancedMeshComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<InstancedMeshComponent>(entity);
            break;
        case ComponentType::Joint2DComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<Joint2DComponent>(entity);
            break;
        case ComponentType::Joint3DComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<Joint3DComponent>(entity);
            break;
        case ComponentType::KeyframeTracksComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<KeyframeTracksComponent>(entity);
            break;
        case ComponentType::LightComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeLightComponent(registry->getComponent<LightComponent>(entity));
            }
            registry->removeComponent<LightComponent>(entity);
            break;
        case ComponentType::LinesComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<LinesComponent>(entity);
            break;
        case ComponentType::MeshPolygonComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<MeshPolygonComponent>(entity);
            break;
        case ComponentType::ModelComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ModelComponent>(entity);
            break;
        case ComponentType::MorphTracksComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<MorphTracksComponent>(entity);
            break;
        case ComponentType::PanelComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<PanelComponent>(entity);
            break;
        case ComponentType::ParticlesComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ParticlesComponent>(entity);
            break;
        case ComponentType::PointsComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<PointsComponent>(entity);
            break;
        case ComponentType::PolygonComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<PolygonComponent>(entity);
            break;
        case ComponentType::PositionActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<PositionActionComponent>(entity);
            break;
        case ComponentType::RotateTracksComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<RotateTracksComponent>(entity);
            break;
        case ComponentType::RotationActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<RotationActionComponent>(entity);
            break;
        case ComponentType::ScaleActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ScaleActionComponent>(entity);
            break;
        case ComponentType::ScaleTracksComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ScaleTracksComponent>(entity);
            break;
        case ComponentType::ScriptComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeScriptComponent(registry->getComponent<ScriptComponent>(entity));
            }
            registry->removeComponent<ScriptComponent>(entity);
            break;
        case ComponentType::ScrollbarComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<ScrollbarComponent>(entity);
            break;
        case ComponentType::SkyComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<SkyComponent>(entity);
            break;
        case ComponentType::SpriteAnimationComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<SpriteAnimationComponent>(entity);
            break;
        case ComponentType::SpriteComponent:
            if (encodeComponent){
                oldComponent = Stream::encodeSpriteComponent(registry->getComponent<SpriteComponent>(entity));
            }
            registry->removeComponent<SpriteComponent>(entity);
            break;
        case ComponentType::TerrainComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TerrainComponent>(entity);
            break;
        case ComponentType::TextComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TextComponent>(entity);
            break;
        case ComponentType::TextEditComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TextEditComponent>(entity);
            break;
        case ComponentType::TilemapComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TilemapComponent>(entity);
            break;
        case ComponentType::TimedActionComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TimedActionComponent>(entity);
            break;
        case ComponentType::TranslateTracksComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<TranslateTracksComponent>(entity);
            break;
        case ComponentType::UIContainerComponent:
            if (encodeComponent){
                Out::error("Missing component serialization of %s", Catalog::getComponentName(componentType).c_str());
            }
            registry->removeComponent<UIContainerComponent>(entity);
            break;
        default:
            break;
    }

    return oldComponent;
}