#include "Catalog.h"

#include "Scene.h"

using namespace Supernova;

Editor::Catalog::Catalog(){
}

std::string Editor::Catalog::getComponentName(ComponentType component){
    if(component == ComponentType::ActionComponent){
        return "ActionComponent";
    }else if(component == ComponentType::AlphaActionComponent){
        return "AlphaActionComponent";
    }else if(component == ComponentType::AnimationComponent){
        return "AnimationComponent";
    }else if(component == ComponentType::AudioComponent){
        return "AudioComponent";
    }else if(component == ComponentType::Body2DComponent){
        return "Body2DComponent";
    }else if(component == ComponentType::Body3DComponent){
        return "Body3DComponent";
    }else if(component == ComponentType::BoneComponent){
        return "BoneComponent";
    }else if(component == ComponentType::ButtonComponent){
        return "ButtonComponent";
    }else if(component == ComponentType::CameraComponent){
        return "CameraComponent";
    }else if(component == ComponentType::ColorActionComponent){
        return "ColorActionComponent";
    }else if(component == ComponentType::FogComponent){
        return "FogComponent";
    }else if(component == ComponentType::ImageComponent){
        return "ImageComponent";
    }else if(component == ComponentType::InstancedMeshComponent){
        return "InstancedMeshComponent";
    }else if(component == ComponentType::Joint2DComponent){
        return "Joint2DComponent";
    }else if(component == ComponentType::Joint3DComponent){
        return "Joint3DComponent";
    }else if(component == ComponentType::KeyframeTracksComponent){
        return "KeyframeTracksComponent";
    }else if(component == ComponentType::LightComponent){
        return "LightComponent";
    }else if(component == ComponentType::LinesComponent){
        return "LinesComponent";
    }else if(component == ComponentType::MeshComponent){
        return "MeshComponent";
    }else if(component == ComponentType::MeshPolygonComponent){
        return "MeshPolygonComponent";
    }else if(component == ComponentType::ModelComponent){
        return "ModelComponent";
    }else if(component == ComponentType::MorphTracksComponent){
        return "MorphTracksComponent";
    }else if(component == ComponentType::PanelComponent){
        return "PanelComponent";
    }else if(component == ComponentType::ParticlesComponent){
        return "ParticlesComponent";
    }else if(component == ComponentType::PointsComponent){
        return "PointsComponent";
    }else if(component == ComponentType::PolygonComponent){
        return "PolygonComponent";
    }else if(component == ComponentType::PositionActionComponent){
        return "PositionActionComponent";
    }else if(component == ComponentType::RotateTracksComponent){
        return "RotateTracksComponent";
    }else if(component == ComponentType::RotationActionComponent){
        return "RotationActionComponent";
    }else if(component == ComponentType::ScaleActionComponent){
        return "ScaleActionComponent";
    }else if(component == ComponentType::ScaleTracksComponent){
        return "ScaleTracksComponent";
    }else if(component == ComponentType::ScrollbarComponent){
        return "ScrollbarComponent";
    }else if(component == ComponentType::SkyComponent){
        return "SkyComponent";
    }else if(component == ComponentType::SpriteAnimationComponent){
        return "SpriteAnimationComponent";
    }else if(component == ComponentType::SpriteComponent){
        return "SpriteComponent";
    }else if(component == ComponentType::TerrainComponent){
        return "TerrainComponent";
    }else if(component == ComponentType::TextComponent){
        return "TextComponent";
    }else if(component == ComponentType::TextEditComponent){
        return "TextEditComponent";
    }else if(component == ComponentType::TilemapComponent){
        return "TilemapComponent";
    }else if(component == ComponentType::TimedActionComponent){
        return "TimedActionComponent";
    }else if(component == ComponentType::Transform){
        return "Transform";
    }else if(component == ComponentType::TranslateTracksComponent){
        return "TranslateTracksComponent";
    }else if(component == ComponentType::UIComponent){
        return "UIComponent";
    }else if(component == ComponentType::UIContainerComponent){
        return "UIContainerComponent";
    }else if(component == ComponentType::UILayoutComponent){
        return "UILayoutComponent";
    }

    return "";
}

std::map<std::string, Editor::PropertyData> Editor::Catalog::getProperties(ComponentType component, void* compRef){
    std::map<std::string, Editor::PropertyData> ps;
    if(component == ComponentType::Transform){
        Transform* comp = (Transform*)compRef;

        ps["position"] = {PropertyType::Float3, "Position", UpdateFlags_Transform, (compRef) ? (void*)&comp->position : nullptr};
        ps["rotation"] = {PropertyType::Quat, "Rotation", UpdateFlags_Transform, (compRef) ? (void*)&comp->rotation : nullptr};
        ps["scale"] = {PropertyType::Float3, "Scale", UpdateFlags_Transform, (compRef) ? (void*)&comp->scale : nullptr};
        ps["visible"] = {PropertyType::Bool, "Visible", UpdateFlags_None, (compRef) ? (void*)&comp->visible : nullptr};
        ps["billboard"] = {PropertyType::Bool, "Billboard", UpdateFlags_Transform, (compRef) ? (void*)&comp->billboard : nullptr};
        ps["fake_billboard"] = {PropertyType::Bool, "Fake", UpdateFlags_Transform, (compRef) ? (void*)&comp->fakeBillboard : nullptr};
        ps["cylindrical_billboard"] = {PropertyType::Bool, "Cylindrical", UpdateFlags_Transform, (compRef) ? (void*)&comp->cylindricalBillboard : nullptr};
        ps["rotation_billboard"] = {PropertyType::Quat, "Rotation", UpdateFlags_Transform, (compRef) ? (void*)&comp->billboardRotation : nullptr};

    }else if (component == ComponentType::MeshComponent){
        MeshComponent* comp = (MeshComponent*)compRef;

        ps["cast_shadows"] = {PropertyType::Bool, "Cast shadows", UpdateFlags_MeshReload, (compRef) ? (void*)&comp->castShadows : nullptr};
        ps["receive_shadows"] = {PropertyType::Bool, "Receive shadows", UpdateFlags_MeshReload, (compRef) ? (void*)&comp->receiveShadows : nullptr};
        ps["num_submeshes"] = {PropertyType::UInt, "Num submesh", UpdateFlags_None, (compRef) ? (void*)&comp->numSubmeshes : nullptr};
        for (int s = 0; s < ((compRef) ? comp->numSubmeshes : 1); s++){
            std::string idx = (compRef) ? std::to_string(s) : "";
            ps["submeshes["+idx+"].primitive_type"] = {PropertyType::PrimitiveType, "Primitive", UpdateFlags_MeshReload, (compRef) ? (void*)&comp->submeshes[s].primitiveType : nullptr};
            ps["submeshes["+idx+"].face_culling"] = {PropertyType::Bool, "Face culling", UpdateFlags_MeshReload, (compRef) ? (void*)&comp->submeshes[s].faceCulling : nullptr};
            ps["submeshes["+idx+"].texture_rect"] = {PropertyType::Float4, "Texture rect", UpdateFlags_None, (compRef) ? (void*)&comp->submeshes[s].textureRect : nullptr};
        }
    }

    return ps;
}

std::vector<Editor::ComponentType> Editor::Catalog::findComponents(Scene* scene, Entity entity){
    std::vector<Editor::ComponentType> ret;

    if (scene->findComponent<Transform>(entity)){
        ret.push_back(ComponentType::Transform);
    }
    if (scene->findComponent<ActionComponent>(entity)){
        ret.push_back(ComponentType::ActionComponent);
    }
    if (scene->findComponent<AlphaActionComponent>(entity)){
        ret.push_back(ComponentType::AlphaActionComponent);
    }
    if (scene->findComponent<AudioComponent>(entity)){
        ret.push_back(ComponentType::AudioComponent);
    }
    if (scene->findComponent<Body2DComponent>(entity)){
        ret.push_back(ComponentType::Body2DComponent);
    }
    if (scene->findComponent<Body3DComponent>(entity)){
        ret.push_back(ComponentType::Body3DComponent);
    }
    if (scene->findComponent<BoneComponent>(entity)){
        ret.push_back(ComponentType::BoneComponent);
    }
    if (scene->findComponent<ButtonComponent>(entity)){
        ret.push_back(ComponentType::ButtonComponent);
    }
    if (scene->findComponent<CameraComponent>(entity)){
        ret.push_back(ComponentType::CameraComponent);
    }
    if (scene->findComponent<ColorActionComponent>(entity)){
        ret.push_back(ComponentType::ColorActionComponent);
    }
    if (scene->findComponent<FogComponent>(entity)){
        ret.push_back(ComponentType::FogComponent);
    }
    if (scene->findComponent<ImageComponent>(entity)){
        ret.push_back(ComponentType::ImageComponent);
    }
    if (scene->findComponent<InstancedMeshComponent>(entity)){
        ret.push_back(ComponentType::InstancedMeshComponent);
    }
    if (scene->findComponent<Joint2DComponent>(entity)){
        ret.push_back(ComponentType::Joint2DComponent);
    }
    if (scene->findComponent<Joint2DComponent>(entity)){
        ret.push_back(ComponentType::Joint2DComponent);
    }
    if (scene->findComponent<KeyframeTracksComponent>(entity)){
        ret.push_back(ComponentType::KeyframeTracksComponent);
    }
    if (scene->findComponent<LightComponent>(entity)){
        ret.push_back(ComponentType::LightComponent);
    }
    if (scene->findComponent<LinesComponent>(entity)){
        ret.push_back(ComponentType::LinesComponent);
    }
    if (scene->findComponent<MeshComponent>(entity)){
        ret.push_back(ComponentType::MeshComponent);
    }
    if (scene->findComponent<MeshPolygonComponent>(entity)){
        ret.push_back(ComponentType::MeshPolygonComponent);
    }
    if (scene->findComponent<ModelComponent>(entity)){
        ret.push_back(ComponentType::ModelComponent);
    }
    if (scene->findComponent<MorphTracksComponent>(entity)){
        ret.push_back(ComponentType::MorphTracksComponent);
    }
    if (scene->findComponent<PanelComponent>(entity)){
        ret.push_back(ComponentType::PanelComponent);
    }
    if (scene->findComponent<ParticlesComponent>(entity)){
        ret.push_back(ComponentType::ParticlesComponent);
    }
    if (scene->findComponent<PointsComponent>(entity)){
        ret.push_back(ComponentType::PointsComponent);
    }
    if (scene->findComponent<PolygonComponent>(entity)){
        ret.push_back(ComponentType::PolygonComponent);
    }
    if (scene->findComponent<PositionActionComponent>(entity)){
        ret.push_back(ComponentType::PositionActionComponent);
    }
    if (scene->findComponent<RotateTracksComponent>(entity)){
        ret.push_back(ComponentType::RotateTracksComponent);
    }
    if (scene->findComponent<RotationActionComponent>(entity)){
        ret.push_back(ComponentType::RotationActionComponent);
    }
    if (scene->findComponent<ScaleActionComponent>(entity)){
        ret.push_back(ComponentType::ScaleActionComponent);
    }
    if (scene->findComponent<ScaleTracksComponent>(entity)){
        ret.push_back(ComponentType::ScaleTracksComponent);
    }
    if (scene->findComponent<ScrollbarComponent>(entity)){
        ret.push_back(ComponentType::ScrollbarComponent);
    }
    if (scene->findComponent<SkyComponent>(entity)){
        ret.push_back(ComponentType::SkyComponent);
    }
    if (scene->findComponent<SpriteAnimationComponent>(entity)){
        ret.push_back(ComponentType::SpriteAnimationComponent);
    }
    if (scene->findComponent<SpriteComponent>(entity)){
        ret.push_back(ComponentType::SpriteComponent);
    }
    if (scene->findComponent<TerrainComponent>(entity)){
        ret.push_back(ComponentType::TerrainComponent);
    }
    if (scene->findComponent<TextComponent>(entity)){
        ret.push_back(ComponentType::TextComponent);
    }
    if (scene->findComponent<TextEditComponent>(entity)){
        ret.push_back(ComponentType::TextEditComponent);
    }
    if (scene->findComponent<TilemapComponent>(entity)){
        ret.push_back(ComponentType::TilemapComponent);
    }
    if (scene->findComponent<TimedActionComponent>(entity)){
        ret.push_back(ComponentType::TimedActionComponent);
    }
    if (scene->findComponent<TranslateTracksComponent>(entity)){
        ret.push_back(ComponentType::TranslateTracksComponent);
    }
    if (scene->findComponent<UIComponent>(entity)){
        ret.push_back(ComponentType::UIComponent);
    }
    if (scene->findComponent<UIContainerComponent>(entity)){
        ret.push_back(ComponentType::UIContainerComponent);
    }
    if (scene->findComponent<UILayoutComponent>(entity)){
        ret.push_back(ComponentType::UILayoutComponent);
    }

    return ret;
}

std::map<std::string, Editor::PropertyData> Editor::Catalog::findEntityProperties(Scene* scene, Entity entity, ComponentType component){
    if(component == ComponentType::Transform){
        if (Transform* compRef = scene->findComponent<Transform>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::MeshComponent){
        if (MeshComponent* compRef = scene->findComponent<MeshComponent>(entity)){
            return getProperties(component, compRef);
        }
    }

    return std::map<std::string, Editor::PropertyData>();
}

std::vector<const char*> Editor::Catalog::getPrimitiveTypeArray(){
    return {"Triangles", "Triangle Strip", "Points", "Lines"};
}

size_t Editor::Catalog::getPrimitiveTypeToIndex(PrimitiveType pt){
    if (pt == PrimitiveType::TRIANGLES){
        return 0;
    }else if (pt == PrimitiveType::TRIANGLE_STRIP){
        return 1;
    }else if (pt == PrimitiveType::POINTS){
        return 2;
    }else if (pt == PrimitiveType::LINES){
        return 3;
    }

    return 0;
}

PrimitiveType Editor::Catalog::getPrimitiveTypeFromIndex(size_t i){
    if (i == 0){
        return PrimitiveType::TRIANGLES;
    }else if (i == 1){
        return PrimitiveType::TRIANGLE_STRIP;
    }else if (i == 2){
        return PrimitiveType::POINTS;
    }else if (i == 3){
        return PrimitiveType::LINES;
    }

    return PrimitiveType::TRIANGLES;
}