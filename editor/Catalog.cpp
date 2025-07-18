#include "Catalog.h"

#include "Scene.h"

using namespace Supernova;

static std::vector<Editor::EnumEntry> entriesPrimitiveType = {
    { (int)PrimitiveType::TRIANGLES, "Triangles" },
    { (int)PrimitiveType::TRIANGLE_STRIP, "Triangle Strip" },
    { (int)PrimitiveType::POINTS, "Points" },
    { (int)PrimitiveType::LINES, "Lines" }
};

static std::vector<Editor::EnumEntry> entriesPivotPreset = {
    { (int)PivotPreset::CENTER, "Center" },
    { (int)PivotPreset::TOP_CENTER, "Top Center" },
    { (int)PivotPreset::BOTTOM_CENTER, "Bottom Center" },
    { (int)PivotPreset::LEFT_CENTER, "Left Center" },
    { (int)PivotPreset::RIGHT_CENTER, "Right Center" },
    { (int)PivotPreset::TOP_LEFT, "Top Left" },
    { (int)PivotPreset::BOTTOM_LEFT, "Bottom Left" },
    { (int)PivotPreset::TOP_RIGHT, "Top Right" },
    { (int)PivotPreset::BOTTOM_RIGHT, "Bottom Right" }
};

static std::vector<Editor::EnumEntry> entriesLightType = {
    { (int)LightType::DIRECTIONAL, "Directional" },
    { (int)LightType::POINT, "Point" },
    { (int)LightType::SPOT, "Spot" }
};

static std::vector<int> cascadeValues = { 1, 2, 3, 4, 5, 6 };
static std::vector<int> po2Values = { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

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
        static Transform* def = new Transform;

        ps["position"] = {PropertyType::Vector3, UpdateFlags_Transform, (void*)&def->position, (compRef) ? (void*)&comp->position : nullptr};
        ps["rotation"] = {PropertyType::Quat, UpdateFlags_Transform, (void*)&def->rotation, (compRef) ? (void*)&comp->rotation : nullptr};
        ps["scale"] = {PropertyType::Vector3, UpdateFlags_Transform, (void*)&def->scale, (compRef) ? (void*)&comp->scale : nullptr};
        ps["visible"] = {PropertyType::Bool, UpdateFlags_None, (void*)&def->visible, (compRef) ? (void*)&comp->visible : nullptr};
        ps["billboard"] = {PropertyType::Bool, UpdateFlags_Transform, (void*)&def->billboard, (compRef) ? (void*)&comp->billboard : nullptr};
        ps["fake_billboard"] = {PropertyType::Bool, UpdateFlags_Transform, (void*)&def->fakeBillboard, (compRef) ? (void*)&comp->fakeBillboard : nullptr};
        ps["cylindrical_billboard"] = {PropertyType::Bool, UpdateFlags_Transform, (void*)&def->cylindricalBillboard, (compRef) ? (void*)&comp->cylindricalBillboard : nullptr};
        ps["rotation_billboard"] = {PropertyType::Quat, UpdateFlags_Transform, (void*)&def->billboardRotation, (compRef) ? (void*)&comp->billboardRotation : nullptr};

    }else if (component == ComponentType::MeshComponent){
        MeshComponent* comp = (MeshComponent*)compRef;
        static MeshComponent* def = new MeshComponent;

        ps["cast_shadows"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->castShadows, (compRef) ? (void*)&comp->castShadows : nullptr};
        ps["receive_shadows"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->receiveShadows, (compRef) ? (void*)&comp->receiveShadows : nullptr};
        ps["num_submeshes"] = {PropertyType::UInt, UpdateFlags_None, (void*)&def->numSubmeshes, (compRef) ? (void*)&comp->numSubmeshes : nullptr};
        for (int s = 0; s < ((compRef) ? MAX_SUBMESHES : 1); s++){
            std::string idx = (compRef) ? std::to_string(s) : "";
            ps["submeshes["+idx+"].material"] = {PropertyType::Material, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material, (compRef) ? (void*)&comp->submeshes[s].material : nullptr};
            ps["submeshes["+idx+"].material.name"] = {PropertyType::String, UpdateFlags_None, (void*)&def->submeshes[0].material.name, (compRef) ? (void*)&comp->submeshes[s].material.name : nullptr};
            ps["submeshes["+idx+"].material.basecolor"] = {PropertyType::Color4L, UpdateFlags_None, (void*)&def->submeshes[0].material.baseColorFactor, (compRef) ? (void*)&comp->submeshes[s].material.baseColorFactor : nullptr};
            ps["submeshes["+idx+"].material.metallicfactor"] = {PropertyType::Float_0_1, UpdateFlags_None, (void*)&def->submeshes[0].material.metallicFactor, (compRef) ? (void*)&comp->submeshes[s].material.metallicFactor : nullptr};
            ps["submeshes["+idx+"].material.roughnessfactor"] = {PropertyType::Float_0_1, UpdateFlags_None, (void*)&def->submeshes[0].material.roughnessFactor, (compRef) ? (void*)&comp->submeshes[s].material.roughnessFactor : nullptr};
            ps["submeshes["+idx+"].material.emissivefactor"] = {PropertyType::Color3L, UpdateFlags_None, (void*)&def->submeshes[0].material.emissiveFactor, (compRef) ? (void*)&comp->submeshes[s].material.emissiveFactor : nullptr};
            ps["submeshes["+idx+"].material.basecolortexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.baseColorTexture, (compRef) ? (void*)&comp->submeshes[s].material.baseColorTexture : nullptr};
            ps["submeshes["+idx+"].material.emissivetexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.emissiveTexture, (compRef) ? (void*)&comp->submeshes[s].material.emissiveTexture : nullptr};
            ps["submeshes["+idx+"].material.metallicroughnesstexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.metallicRoughnessTexture, (compRef) ? (void*)&comp->submeshes[s].material.metallicRoughnessTexture : nullptr};
            ps["submeshes["+idx+"].material.occlusiontexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.occlusionTexture, (compRef) ? (void*)&comp->submeshes[s].material.occlusionTexture : nullptr};
            ps["submeshes["+idx+"].material.normalTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.normalTexture, (compRef) ? (void*)&comp->submeshes[s].material.normalTexture : nullptr};

            ps["submeshes["+idx+"].primitive_type"] = {PropertyType::Enum, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].primitiveType, (compRef) ? (void*)&comp->submeshes[s].primitiveType : nullptr, &entriesPrimitiveType};
            ps["submeshes["+idx+"].face_culling"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].faceCulling, (compRef) ? (void*)&comp->submeshes[s].faceCulling : nullptr};
            ps["submeshes["+idx+"].texture_shadow"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].textureShadow, (compRef) ? (void*)&comp->submeshes[s].textureShadow : nullptr};
            ps["submeshes["+idx+"].texture_rect"] = {PropertyType::Vector4, UpdateFlags_None, (void*)&def->submeshes[0].textureRect, (compRef) ? (void*)&comp->submeshes[s].textureRect : nullptr};
        }
    }else if (component == ComponentType::UIComponent){
        UIComponent* comp = (UIComponent*)compRef;
        static UIComponent* def = new UIComponent;

        ps["color"] = {PropertyType::Color4L, UpdateFlags_None, (void*)&def->color, (compRef) ? (void*)&comp->color : nullptr};
        ps["texture"] = {PropertyType::Texture, UpdateFlags_UI_Texture, (void*)&def->texture, (compRef) ? (void*)&comp->texture : nullptr};
    }else if (component == ComponentType::UILayoutComponent){
        UILayoutComponent* comp = (UILayoutComponent*)compRef;
        static UILayoutComponent* def = new UILayoutComponent;

        ps["width"] = {PropertyType::UInt, UpdateFlags_Layout_Sizes, nullptr, (compRef) ? (void*)&comp->width : nullptr};
        ps["height"] = {PropertyType::UInt, UpdateFlags_Layout_Sizes, nullptr, (compRef) ? (void*)&comp->height : nullptr};
        ps["ignore_scissor"] = {PropertyType::Bool, UpdateFlags_None, (void*)&def->ignoreScissor, (compRef) ? (void*)&comp->ignoreScissor : nullptr};
    }else if (component == ComponentType::ImageComponent){
        ImageComponent* comp = (ImageComponent*)compRef;
        static ImageComponent* def = new ImageComponent;

        ps["patch_margin_left"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginLeft, (compRef) ? (void*)&comp->patchMarginLeft : nullptr};
        ps["patch_margin_right"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginRight, (compRef) ? (void*)&comp->patchMarginRight : nullptr};
        ps["patch_margin_top"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginTop, (compRef) ? (void*)&comp->patchMarginTop : nullptr};
        ps["patch_margin_bottom"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginBottom, (compRef) ? (void*)&comp->patchMarginBottom : nullptr};
        ps["texture_scale_factor"] = {PropertyType::Float, UpdateFlags_Image_Patches, (void*)&def->textureScaleFactor, (compRef) ? (void*)&comp->textureScaleFactor : nullptr};
    }else if (component == ComponentType::SpriteComponent){
        SpriteComponent* comp = (SpriteComponent*)compRef;
        static SpriteComponent* def = new SpriteComponent;

        ps["width"] = {PropertyType::UInt, UpdateFlags_Sprite, nullptr, (compRef) ? (void*)&comp->width : nullptr};
        ps["height"] = {PropertyType::UInt, UpdateFlags_Sprite, nullptr, (compRef) ? (void*)&comp->height : nullptr};
        ps["pivot_preset"] = {PropertyType::Enum, UpdateFlags_Sprite, (void*)&def->pivotPreset, (compRef) ? (void*)&comp->pivotPreset : nullptr, &entriesPivotPreset};
        ps["texture_scale_factor"] = {PropertyType::Float, UpdateFlags_Sprite, (void*)&def->textureScaleFactor, (compRef) ? (void*)&comp->textureScaleFactor : nullptr};
    }else if (component == ComponentType::LightComponent){
        LightComponent* comp = (LightComponent*)compRef;
        static LightComponent* def = new LightComponent;

        ps["type"] = {PropertyType::Enum, UpdateFlags_LightShadowMap | UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, nullptr, (compRef) ? (void*)&comp->type : nullptr, &entriesLightType};
        ps["direction"] = {PropertyType::Direction, UpdateFlags_Transform, (void*)&def->direction, (compRef) ? (void*)&comp->direction : nullptr};
        ps["shadows"] = {PropertyType::Bool, UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, (void*)&def->shadows, (compRef) ? (void*)&comp->shadows : nullptr};
        ps["intensity"] = {PropertyType::Float, UpdateFlags_None, (void*)&def->intensity, (compRef) ? (void*)&comp->intensity : nullptr};
        ps["range"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, (void*)&def->range, (compRef) ? (void*)&comp->range : nullptr};
        ps["color"] = {PropertyType::Color3L, UpdateFlags_None, (void*)&def->color, (compRef) ? (void*)&comp->color : nullptr};
        ps["inner_cone_cos"] = {PropertyType::HalfCone, UpdateFlags_LightShadowCamera, (void*)&def->innerConeCos, (compRef) ? (void*)&comp->innerConeCos : nullptr};
        ps["outer_cone_cos"] = {PropertyType::HalfCone, UpdateFlags_LightShadowCamera, (void*)&def->outerConeCos, (compRef) ? (void*)&comp->outerConeCos : nullptr};
        ps["shadow_bias"] = {PropertyType::Float, UpdateFlags_None, (void*)&def->shadowBias, (compRef) ? (void*)&comp->shadowBias : nullptr};
        ps["map_resolution"] = {PropertyType::UIntSlider, UpdateFlags_LightShadowMap | UpdateFlags_Scene_Mesh_Reload, (void*)&def->mapResolution, (compRef) ? (void*)&comp->mapResolution : nullptr, nullptr, &po2Values};
        ps["automatic_shadow_camera"] = {PropertyType::Bool, UpdateFlags_LightShadowCamera, (void*)&def->automaticShadowCamera, (compRef) ? (void*)&comp->automaticShadowCamera : nullptr};
        ps["shadow_camera_near"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, nullptr, (compRef) ? (void*)&comp->shadowCameraNearFar.x : nullptr};
        ps["shadow_camera_far"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, nullptr, (compRef) ? (void*)&comp->shadowCameraNearFar.y : nullptr};
        ps["num_shadow_cascades"] = {PropertyType::UIntSlider, UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, (void*)&def->numShadowCascades, (compRef) ? (void*)&comp->numShadowCascades : nullptr, nullptr, &cascadeValues};
    }

    return ps;
}

std::vector<Editor::ComponentType> Editor::Catalog::findComponents(Scene* scene, Entity entity){
    std::vector<Editor::ComponentType> ret;

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
    if (scene->findComponent<Joint3DComponent>(entity)){
        ret.push_back(ComponentType::Joint3DComponent);
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
    if (scene->findComponent<Transform>(entity)){
        ret.push_back(ComponentType::Transform);
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
    }else if (component == ComponentType::UIComponent){
        if (UIComponent* compRef = scene->findComponent<UIComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::UILayoutComponent){
        if (UILayoutComponent* compRef = scene->findComponent<UILayoutComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::ImageComponent){
        if (ImageComponent* compRef = scene->findComponent<ImageComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::SpriteComponent){
        if (SpriteComponent* compRef = scene->findComponent<SpriteComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::LightComponent){
        if (LightComponent* compRef = scene->findComponent<LightComponent>(entity)){
            return getProperties(component, compRef);
        }
    }

    return std::map<std::string, Editor::PropertyData>();
}

void Editor::Catalog::updateEntity(Scene* scene, Entity entity, int updateFlags){
    if (updateFlags & UpdateFlags_Transform){
        scene->getComponent<Transform>(entity).needUpdate = true;
    }
    if (updateFlags & UpdateFlags_Scene_Mesh_Reload){
        auto meshes = scene->getComponentArray<MeshComponent>();
        for (int i = 0; i < meshes->size(); i++) {
            MeshComponent& mesh = meshes->getComponentFromIndex(i);
            mesh.needReload = true;
        }
    }
    if (updateFlags & UpdateFlags_Mesh_Reload){
        scene->getComponent<MeshComponent>(entity).needReload = true;
    }
    if (updateFlags & UpdateFlags_Mesh_Texture){
        unsigned int numSubmeshes = scene->getComponent<MeshComponent>(entity).numSubmeshes;
        for (unsigned int i = 0; i < numSubmeshes; i++){
            scene->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
        }
    }
    if (updateFlags & UpdateFlags_LightShadowMap){
        scene->getComponent<LightComponent>(entity).needUpdateShadowMap = true;
    }
    if (updateFlags & UpdateFlags_LightShadowCamera){
        scene->getComponent<LightComponent>(entity).needUpdateShadowCamera = true;
    }
    if (updateFlags & UpdateFlags_UI_Reload){
        scene->getComponent<UIComponent>(entity).needReload = true;
    }
    if (updateFlags & UpdateFlags_UI_Texture){
        scene->getComponent<UIComponent>(entity).needUpdateTexture = true;
    }
    if (updateFlags & UpdateFlags_Image_Patches){
        scene->getComponent<ImageComponent>(entity).needUpdatePatches = true;
    }
    if (updateFlags & UpdateFlags_Layout_Sizes){
        scene->getComponent<UILayoutComponent>(entity).needUpdateSizes = true;
    }
    if (updateFlags & UpdateFlags_Sprite){
        scene->getComponent<SpriteComponent>(entity).needUpdateSprite = true;
    }
}

void Editor::Catalog::copyPropertyValue(Scene* sourceScene, Entity sourceEntity, 
                                       Scene* targetScene, Entity targetEntity, 
                                       ComponentType compType, const std::string& property) {

    // Get the property data to determine the type
    auto sourceProperties = Catalog::findEntityProperties(sourceScene, sourceEntity, compType);
    auto propIt = sourceProperties.find(property);
    if (propIt == sourceProperties.end()) {
        return; // Property not found
    }

    PropertyType propType = propIt->second.type;

    // Copy based on property type
    switch (propType) {
        case PropertyType::Bool: {
            bool* source = Catalog::getPropertyRef<bool>(sourceScene, sourceEntity, compType, property);
            bool* target = Catalog::getPropertyRef<bool>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Float:
        case PropertyType::Float_0_1: {
            float* source = Catalog::getPropertyRef<float>(sourceScene, sourceEntity, compType, property);
            float* target = Catalog::getPropertyRef<float>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Int: {
            int* source = Catalog::getPropertyRef<int>(sourceScene, sourceEntity, compType, property);
            int* target = Catalog::getPropertyRef<int>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::UInt:
        case PropertyType::UIntSlider: {
            unsigned int* source = Catalog::getPropertyRef<unsigned int>(sourceScene, sourceEntity, compType, property);
            unsigned int* target = Catalog::getPropertyRef<unsigned int>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector2: {
            Vector2* source = Catalog::getPropertyRef<Vector2>(sourceScene, sourceEntity, compType, property);
            Vector2* target = Catalog::getPropertyRef<Vector2>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector3:
        case PropertyType::Color3L:
        case PropertyType::Direction: {
            Vector3* source = Catalog::getPropertyRef<Vector3>(sourceScene, sourceEntity, compType, property);
            Vector3* target = Catalog::getPropertyRef<Vector3>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector4:
        case PropertyType::Color4L: {
            Vector4* source = Catalog::getPropertyRef<Vector4>(sourceScene, sourceEntity, compType, property);
            Vector4* target = Catalog::getPropertyRef<Vector4>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Quat: {
            Quaternion* source = Catalog::getPropertyRef<Quaternion>(sourceScene, sourceEntity, compType, property);
            Quaternion* target = Catalog::getPropertyRef<Quaternion>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::String: {
            std::string* source = Catalog::getPropertyRef<std::string>(sourceScene, sourceEntity, compType, property);
            std::string* target = Catalog::getPropertyRef<std::string>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Material: {
            Material* source = Catalog::getPropertyRef<Material>(sourceScene, sourceEntity, compType, property);
            Material* target = Catalog::getPropertyRef<Material>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Texture: {
            Texture* source = Catalog::getPropertyRef<Texture>(sourceScene, sourceEntity, compType, property);
            Texture* target = Catalog::getPropertyRef<Texture>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::HalfCone: {
            float* source = Catalog::getPropertyRef<float>(sourceScene, sourceEntity, compType, property);
            float* target = Catalog::getPropertyRef<float>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Enum: {
            int* source = Catalog::getPropertyRef<int>(sourceScene, sourceEntity, compType, property);
            int* target = Catalog::getPropertyRef<int>(targetScene, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        default:
            // For any unknown/unsupported type, do nothing
            break;
    }

    // Apply any update flags that are associated with this property
    updateEntity(targetScene, targetEntity, propIt->second.updateFlags);
}