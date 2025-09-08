#include "RemoveComponentCmd.h"

#include "Stream.h"

using namespace Supernova;

Editor::RemoveComponentCmd::RemoveComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType){
    this->project = project;
    this->sceneId = sceneId;
    this->entity = entity;
    this->componentType = componentType;
}

bool Editor::RemoveComponentCmd::execute() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        switch (componentType) {
            case ComponentType::Transform:
                oldComponent = Stream::encodeTransform(scene->getComponent<Transform>(entity));
                scene->removeComponent<Transform>(entity);
                break;
            case ComponentType::MeshComponent:
                oldComponent = Stream::encodeMeshComponent(scene->getComponent<MeshComponent>(entity));
                scene->removeComponent<MeshComponent>(entity);
                break;
            case ComponentType::UIComponent:
                oldComponent = Stream::encodeMeshComponent(scene->getComponent<MeshComponent>(entity));
                scene->removeComponent<UIComponent>(entity);
                break;
            case ComponentType::UILayoutComponent:
                oldComponent = Stream::encodeUILayoutComponent(scene->getComponent<UILayoutComponent>(entity));
                scene->removeComponent<UILayoutComponent>(entity);
                break;
            case ComponentType::ActionComponent:
                scene->removeComponent<ActionComponent>(entity);
                break;
            case ComponentType::AlphaActionComponent:
                scene->removeComponent<AlphaActionComponent>(entity);
                break;
            case ComponentType::AnimationComponent:
                scene->removeComponent<AnimationComponent>(entity);
                break;
            case ComponentType::AudioComponent:
                scene->removeComponent<AudioComponent>(entity);
                break;
            case ComponentType::Body2DComponent:
                scene->removeComponent<Body2DComponent>(entity);
                break;
            case ComponentType::Body3DComponent:
                scene->removeComponent<Body3DComponent>(entity);
                break;
            case ComponentType::BoneComponent:
                scene->removeComponent<BoneComponent>(entity);
                break;
            case ComponentType::ButtonComponent:
                scene->removeComponent<ButtonComponent>(entity);
                break;
            case ComponentType::CameraComponent:
                scene->removeComponent<CameraComponent>(entity);
                break;
            case ComponentType::ColorActionComponent:
                scene->removeComponent<ColorActionComponent>(entity);
                break;
            case ComponentType::FogComponent:
                scene->removeComponent<FogComponent>(entity);
                break;
            case ComponentType::ImageComponent:
                oldComponent = Stream::encodeImageComponent(scene->getComponent<ImageComponent>(entity));
                scene->removeComponent<ImageComponent>(entity);
                break;
            case ComponentType::InstancedMeshComponent:
                scene->removeComponent<InstancedMeshComponent>(entity);
                break;
            case ComponentType::Joint2DComponent:
                scene->removeComponent<Joint2DComponent>(entity);
                break;
            case ComponentType::Joint3DComponent:
                scene->removeComponent<Joint3DComponent>(entity);
                break;
            case ComponentType::KeyframeTracksComponent:
                scene->removeComponent<KeyframeTracksComponent>(entity);
                break;
            case ComponentType::LightComponent:
                oldComponent = Stream::encodeLightComponent(scene->getComponent<LightComponent>(entity));
                scene->removeComponent<LightComponent>(entity);
                break;
            case ComponentType::LinesComponent:
                scene->removeComponent<LinesComponent>(entity);
                break;
            case ComponentType::MeshPolygonComponent:
                scene->removeComponent<MeshPolygonComponent>(entity);
                break;
            case ComponentType::ModelComponent:
                scene->removeComponent<ModelComponent>(entity);
                break;
            case ComponentType::MorphTracksComponent:
                scene->removeComponent<MorphTracksComponent>(entity);
                break;
            case ComponentType::PanelComponent:
                scene->removeComponent<PanelComponent>(entity);
                break;
            case ComponentType::ParticlesComponent:
                scene->removeComponent<ParticlesComponent>(entity);
                break;
            case ComponentType::PointsComponent:
                scene->removeComponent<PointsComponent>(entity);
                break;
            case ComponentType::PolygonComponent:
                scene->removeComponent<PolygonComponent>(entity);
                break;
            case ComponentType::PositionActionComponent:
                scene->removeComponent<PositionActionComponent>(entity);
                break;
            case ComponentType::RotateTracksComponent:
                scene->removeComponent<RotateTracksComponent>(entity);
                break;
            case ComponentType::RotationActionComponent:
                scene->removeComponent<RotationActionComponent>(entity);
                break;
            case ComponentType::ScaleActionComponent:
                scene->removeComponent<ScaleActionComponent>(entity);
                break;
            case ComponentType::ScaleTracksComponent:
                scene->removeComponent<ScaleTracksComponent>(entity);
                break;
            case ComponentType::ScriptComponent:
                scene->removeComponent<ScriptComponent>(entity);
                break;
            case ComponentType::ScrollbarComponent:
                scene->removeComponent<ScrollbarComponent>(entity);
                break;
            case ComponentType::SkyComponent:
                scene->removeComponent<SkyComponent>(entity);
                break;
            case ComponentType::SpriteAnimationComponent:
                scene->removeComponent<SpriteAnimationComponent>(entity);
                break;
            case ComponentType::SpriteComponent:
                scene->removeComponent<SpriteComponent>(entity);
                break;
            case ComponentType::TerrainComponent:
                scene->removeComponent<TerrainComponent>(entity);
                break;
            case ComponentType::TextComponent:
                scene->removeComponent<TextComponent>(entity);
                break;
            case ComponentType::TextEditComponent:
                scene->removeComponent<TextEditComponent>(entity);
                break;
            case ComponentType::TilemapComponent:
                scene->removeComponent<TilemapComponent>(entity);
                break;
            case ComponentType::TimedActionComponent:
                scene->removeComponent<TimedActionComponent>(entity);
                break;
            case ComponentType::TranslateTracksComponent:
                scene->removeComponent<TranslateTracksComponent>(entity);
                break;
            case ComponentType::UIContainerComponent:
                scene->removeComponent<UIContainerComponent>(entity);
                break;
            default:
                break;
        }

        sceneProject->isModified = true;
    }

    return true;
}

void Editor::RemoveComponentCmd::undo() {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        Scene* scene = sceneProject->scene;

        switch (componentType) {
            case ComponentType::Transform:
                scene->addComponent<Transform>(entity, Stream::decodeTransform(oldComponent));
                break;
            case ComponentType::MeshComponent:
                scene->addComponent<MeshComponent>(entity, Stream::decodeMeshComponent(oldComponent));
                break;
            case ComponentType::UIComponent:
                scene->addComponent<UIComponent>(entity, Stream::decodeUIComponent(oldComponent));
                break;
            case ComponentType::UILayoutComponent:
                scene->addComponent<UILayoutComponent>(entity, Stream::decodeUILayoutComponent(oldComponent));
                break;
            case ComponentType::ActionComponent:
                scene->addComponent<ActionComponent>(entity, {});
                break;
            case ComponentType::AlphaActionComponent:
                scene->addComponent<AlphaActionComponent>(entity, {});
                break;
            case ComponentType::AnimationComponent:
                scene->addComponent<AnimationComponent>(entity, {});
                break;
            case ComponentType::AudioComponent:
                scene->addComponent<AudioComponent>(entity, {});
                break;
            case ComponentType::Body2DComponent:
                scene->addComponent<Body2DComponent>(entity, {});
                break;
            case ComponentType::Body3DComponent:
                scene->addComponent<Body3DComponent>(entity, {});
                break;
            case ComponentType::BoneComponent:
                scene->addComponent<BoneComponent>(entity, {});
                break;
            case ComponentType::ButtonComponent:
                scene->addComponent<ButtonComponent>(entity, {});
                break;
            case ComponentType::CameraComponent:
                scene->addComponent<CameraComponent>(entity, {});
                break;
            case ComponentType::ColorActionComponent:
                scene->addComponent<ColorActionComponent>(entity, {});
                break;
            case ComponentType::FogComponent:
                scene->addComponent<FogComponent>(entity, {});
                break;
            case ComponentType::ImageComponent:
                scene->addComponent<ImageComponent>(entity, Stream::decodeImageComponent(oldComponent));
                break;
            case ComponentType::InstancedMeshComponent:
                scene->addComponent<InstancedMeshComponent>(entity, {});
                break;
            case ComponentType::Joint2DComponent:
                scene->addComponent<Joint2DComponent>(entity, {});
                break;
            case ComponentType::Joint3DComponent:
                scene->addComponent<Joint3DComponent>(entity, {});
                break;
            case ComponentType::KeyframeTracksComponent:
                scene->addComponent<KeyframeTracksComponent>(entity, {});
                break;
            case ComponentType::LightComponent:
                scene->addComponent<LightComponent>(entity, Stream::decodeLightComponent(oldComponent));
                break;
            case ComponentType::LinesComponent:
                scene->addComponent<LinesComponent>(entity, {});
                break;
            case ComponentType::MeshPolygonComponent:
                scene->addComponent<MeshPolygonComponent>(entity, {});
                break;
            case ComponentType::ModelComponent:
                scene->addComponent<ModelComponent>(entity, {});
                break;
            case ComponentType::MorphTracksComponent:
                scene->addComponent<MorphTracksComponent>(entity, {});
                break;
            case ComponentType::PanelComponent:
                scene->addComponent<PanelComponent>(entity, {});
                break;
            case ComponentType::ParticlesComponent:
                scene->addComponent<ParticlesComponent>(entity, {});
                break;
            case ComponentType::PointsComponent:
                scene->addComponent<PointsComponent>(entity, {});
                break;
            case ComponentType::PolygonComponent:
                scene->addComponent<PolygonComponent>(entity, {});
                break;
            case ComponentType::PositionActionComponent:
                scene->addComponent<PositionActionComponent>(entity, {});
                break;
            case ComponentType::RotateTracksComponent:
                scene->addComponent<RotateTracksComponent>(entity, {});
                break;
            case ComponentType::RotationActionComponent:
                scene->addComponent<RotationActionComponent>(entity, {});
                break;
            case ComponentType::ScaleActionComponent:
                scene->addComponent<ScaleActionComponent>(entity, {});
                break;
            case ComponentType::ScaleTracksComponent:
                scene->addComponent<ScaleTracksComponent>(entity, {});
                break;
            case ComponentType::ScriptComponent:
                scene->addComponent<ScriptComponent>(entity, {});
                break;
            case ComponentType::ScrollbarComponent:
                scene->addComponent<ScrollbarComponent>(entity, {});
                break;
            case ComponentType::SkyComponent:
                scene->addComponent<SkyComponent>(entity, {});
                break;
            case ComponentType::SpriteAnimationComponent:
                scene->addComponent<SpriteAnimationComponent>(entity, {});
                break;
            case ComponentType::SpriteComponent:
                scene->addComponent<SpriteComponent>(entity, {});
                break;
            case ComponentType::TerrainComponent:
                scene->addComponent<TerrainComponent>(entity, {});
                break;
            case ComponentType::TextComponent:
                scene->addComponent<TextComponent>(entity, {});
                break;
            case ComponentType::TextEditComponent:
                scene->addComponent<TextEditComponent>(entity, {});
                break;
            case ComponentType::TilemapComponent:
                scene->addComponent<TilemapComponent>(entity, {});
                break;
            case ComponentType::TimedActionComponent:
                scene->addComponent<TimedActionComponent>(entity, {});
                break;
            case ComponentType::TranslateTracksComponent:
                scene->addComponent<TranslateTracksComponent>(entity, {});
                break;
            case ComponentType::UIContainerComponent:
                scene->addComponent<UIContainerComponent>(entity, {});
                break;
            default:
                break;
        }

        sceneProject->isModified = true;
    }
}

bool Editor::RemoveComponentCmd::mergeWith(Command* otherCommand){

    return false;
}