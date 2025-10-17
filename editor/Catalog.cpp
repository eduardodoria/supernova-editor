#include "Catalog.h"

#include "Scene.h"
#include "yaml-cpp/yaml.h"
#include "Stream.h"

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

std::string Editor::Catalog::removeComponentSuffix(std::string str) {
    // Convert to lowercase first
    std::transform(str.begin(), str.end(), str.begin(), 
                   [](unsigned char c) { return std::tolower(c); });

    // Check if string ends with "component"
    const std::string suffix = "component";
    if (str.length() >= suffix.length() && 
        str.substr(str.length() - suffix.length()) == suffix) {
        str.erase(str.length() - suffix.length());
    }

    return str;
}

std::string Editor::Catalog::getComponentName(ComponentType component, bool removeSuffix){
    std::string name;

    if(component == ComponentType::ActionComponent){
        name = "ActionComponent";
    }else if(component == ComponentType::AlphaActionComponent){
        name = "AlphaActionComponent";
    }else if(component == ComponentType::AnimationComponent){
        name = "AnimationComponent";
    }else if(component == ComponentType::AudioComponent){
        name = "AudioComponent";
    }else if(component == ComponentType::Body2DComponent){
        name = "Body2DComponent";
    }else if(component == ComponentType::Body3DComponent){
        name = "Body3DComponent";
    }else if(component == ComponentType::BoneComponent){
        name = "BoneComponent";
    }else if(component == ComponentType::ButtonComponent){
        name = "ButtonComponent";
    }else if(component == ComponentType::CameraComponent){
        name = "CameraComponent";
    }else if(component == ComponentType::ColorActionComponent){
        name = "ColorActionComponent";
    }else if(component == ComponentType::FogComponent){
        name = "FogComponent";
    }else if(component == ComponentType::ImageComponent){
        name = "ImageComponent";
    }else if(component == ComponentType::InstancedMeshComponent){
        name = "InstancedMeshComponent";
    }else if(component == ComponentType::Joint2DComponent){
        name = "Joint2DComponent";
    }else if(component == ComponentType::Joint3DComponent){
        name = "Joint3DComponent";
    }else if(component == ComponentType::KeyframeTracksComponent){
        name = "KeyframeTracksComponent";
    }else if(component == ComponentType::LightComponent){
        name = "LightComponent";
    }else if(component == ComponentType::LinesComponent){
        name = "LinesComponent";
    }else if(component == ComponentType::MeshComponent){
        name = "MeshComponent";
    }else if(component == ComponentType::MeshPolygonComponent){
        name = "MeshPolygonComponent";
    }else if(component == ComponentType::ModelComponent){
        name = "ModelComponent";
    }else if(component == ComponentType::MorphTracksComponent){
        name = "MorphTracksComponent";
    }else if(component == ComponentType::PanelComponent){
        name = "PanelComponent";
    }else if(component == ComponentType::ParticlesComponent){
        name = "ParticlesComponent";
    }else if(component == ComponentType::PointsComponent){
        name = "PointsComponent";
    }else if(component == ComponentType::PolygonComponent){
        name = "PolygonComponent";
    }else if(component == ComponentType::PositionActionComponent){
        name = "PositionActionComponent";
    }else if(component == ComponentType::RotateTracksComponent){
        name = "RotateTracksComponent";
    }else if(component == ComponentType::RotationActionComponent){
        name = "RotationActionComponent";
    }else if(component == ComponentType::ScaleActionComponent){
        name = "ScaleActionComponent";
    }else if(component == ComponentType::ScaleTracksComponent){
        name = "ScaleTracksComponent";
    }else if(component == ComponentType::ScriptComponent){
        name = "ScriptComponent";
    }else if(component == ComponentType::ScrollbarComponent){
        name = "ScrollbarComponent";
    }else if(component == ComponentType::SkyComponent){
        name = "SkyComponent";
    }else if(component == ComponentType::SpriteAnimationComponent){
        name = "SpriteAnimationComponent";
    }else if(component == ComponentType::SpriteComponent){
        name = "SpriteComponent";
    }else if(component == ComponentType::TerrainComponent){
        name = "TerrainComponent";
    }else if(component == ComponentType::TextComponent){
        name = "TextComponent";
    }else if(component == ComponentType::TextEditComponent){
        name = "TextEditComponent";
    }else if(component == ComponentType::TilemapComponent){
        name = "TilemapComponent";
    }else if(component == ComponentType::TimedActionComponent){
        name = "TimedActionComponent";
    }else if(component == ComponentType::Transform){
        name = "Transform";
    }else if(component == ComponentType::TranslateTracksComponent){
        name = "TranslateTracksComponent";
    }else if(component == ComponentType::UIComponent){
        name = "UIComponent";
    }else if(component == ComponentType::UIContainerComponent){
        name = "UIContainerComponent";
    }else if(component == ComponentType::UILayoutComponent){
        name = "UILayoutComponent";
    }else{
        return "";
    }

    if(removeSuffix){
        return removeComponentSuffix(name);
    }

    return name;
}

ComponentId Editor::Catalog::getComponentId(const EntityRegistry* registry, ComponentType compType) {
    using namespace Supernova;
    switch (compType) {
        case ComponentType::Transform:
            return registry->getComponentId<Transform>();
        case ComponentType::MeshComponent:
            return registry->getComponentId<MeshComponent>();
        case ComponentType::UIComponent:
            return registry->getComponentId<UIComponent>();
        case ComponentType::UILayoutComponent:
            return registry->getComponentId<UILayoutComponent>();
        case ComponentType::ActionComponent:
            return registry->getComponentId<ActionComponent>();
        case ComponentType::AlphaActionComponent:
            return registry->getComponentId<AlphaActionComponent>();
        case ComponentType::AnimationComponent:
            return registry->getComponentId<AnimationComponent>();
        case ComponentType::AudioComponent:
            return registry->getComponentId<AudioComponent>();
        case ComponentType::Body2DComponent:
            return registry->getComponentId<Body2DComponent>();
        case ComponentType::Body3DComponent:
            return registry->getComponentId<Body3DComponent>();
        case ComponentType::BoneComponent:
            return registry->getComponentId<BoneComponent>();
        case ComponentType::ButtonComponent:
            return registry->getComponentId<ButtonComponent>();
        case ComponentType::CameraComponent:
            return registry->getComponentId<CameraComponent>();
        case ComponentType::ColorActionComponent:
            return registry->getComponentId<ColorActionComponent>();
        case ComponentType::FogComponent:
            return registry->getComponentId<FogComponent>();
        case ComponentType::ImageComponent:
            return registry->getComponentId<ImageComponent>();
        case ComponentType::InstancedMeshComponent:
            return registry->getComponentId<InstancedMeshComponent>();
        case ComponentType::Joint2DComponent:
            return registry->getComponentId<Joint2DComponent>();
        case ComponentType::Joint3DComponent:
            return registry->getComponentId<Joint3DComponent>();
        case ComponentType::KeyframeTracksComponent:
            return registry->getComponentId<KeyframeTracksComponent>();
        case ComponentType::LightComponent:
            return registry->getComponentId<LightComponent>();
        case ComponentType::LinesComponent:
            return registry->getComponentId<LinesComponent>();
        case ComponentType::MeshPolygonComponent:
            return registry->getComponentId<MeshPolygonComponent>();
        case ComponentType::ModelComponent:
            return registry->getComponentId<ModelComponent>();
        case ComponentType::MorphTracksComponent:
            return registry->getComponentId<MorphTracksComponent>();
        case ComponentType::PanelComponent:
            return registry->getComponentId<PanelComponent>();
        case ComponentType::ParticlesComponent:
            return registry->getComponentId<ParticlesComponent>();
        case ComponentType::PointsComponent:
            return registry->getComponentId<PointsComponent>();
        case ComponentType::PolygonComponent:
            return registry->getComponentId<PolygonComponent>();
        case ComponentType::PositionActionComponent:
            return registry->getComponentId<PositionActionComponent>();
        case ComponentType::RotateTracksComponent:
            return registry->getComponentId<RotateTracksComponent>();
        case ComponentType::RotationActionComponent:
            return registry->getComponentId<RotationActionComponent>();
        case ComponentType::ScaleActionComponent:
            return registry->getComponentId<ScaleActionComponent>();
        case ComponentType::ScaleTracksComponent:
            return registry->getComponentId<ScaleTracksComponent>();
        case ComponentType::ScriptComponent:
            return registry->getComponentId<ScriptComponent>();
        case ComponentType::ScrollbarComponent:
            return registry->getComponentId<ScrollbarComponent>();
        case ComponentType::SkyComponent:
            return registry->getComponentId<SkyComponent>();
        case ComponentType::SpriteAnimationComponent:
            return registry->getComponentId<SpriteAnimationComponent>();
        case ComponentType::SpriteComponent:
            return registry->getComponentId<SpriteComponent>();
        case ComponentType::TerrainComponent:
            return registry->getComponentId<TerrainComponent>();
        case ComponentType::TextComponent:
            return registry->getComponentId<TextComponent>();
        case ComponentType::TextEditComponent:
            return registry->getComponentId<TextEditComponent>();
        case ComponentType::TilemapComponent:
            return registry->getComponentId<TilemapComponent>();
        case ComponentType::TimedActionComponent:
            return registry->getComponentId<TimedActionComponent>();
        case ComponentType::TranslateTracksComponent:
            return registry->getComponentId<TranslateTracksComponent>();
        case ComponentType::UIContainerComponent:
            return registry->getComponentId<UIContainerComponent>();
        default:
            return 0;
    }
}

Editor::ComponentType Editor::Catalog::getComponentType(const std::string& componentName) {
    std::string normalizedName = removeComponentSuffix(componentName);

    if(normalizedName == "action"){
        return ComponentType::ActionComponent;
    }else if(normalizedName == "alphaaction"){
        return ComponentType::AlphaActionComponent;
    }else if(normalizedName == "animation"){
        return ComponentType::AnimationComponent;
    }else if(normalizedName == "audio"){
        return ComponentType::AudioComponent;
    }else if(normalizedName == "body2d"){
        return ComponentType::Body2DComponent;
    }else if(normalizedName == "body3d"){
        return ComponentType::Body3DComponent;
    }else if(normalizedName == "bone"){
        return ComponentType::BoneComponent;
    }else if(normalizedName == "button"){
        return ComponentType::ButtonComponent;
    }else if(normalizedName == "camera"){
        return ComponentType::CameraComponent;
    }else if(normalizedName == "coloraction"){
        return ComponentType::ColorActionComponent;
    }else if(normalizedName == "fog"){
        return ComponentType::FogComponent;
    }else if(normalizedName == "image"){
        return ComponentType::ImageComponent;
    }else if(normalizedName == "instancedmesh"){
        return ComponentType::InstancedMeshComponent;
    }else if(normalizedName == "joint2d"){
        return ComponentType::Joint2DComponent;
    }else if(normalizedName == "joint3d"){
        return ComponentType::Joint3DComponent;
    }else if(normalizedName == "keyframetracks"){
        return ComponentType::KeyframeTracksComponent;
    }else if(normalizedName == "light"){
        return ComponentType::LightComponent;
    }else if(normalizedName == "lines"){
        return ComponentType::LinesComponent;
    }else if(normalizedName == "mesh"){
        return ComponentType::MeshComponent;
    }else if(normalizedName == "meshpolygon"){
        return ComponentType::MeshPolygonComponent;
    }else if(normalizedName == "model"){
        return ComponentType::ModelComponent;
    }else if(normalizedName == "morphtracks"){
        return ComponentType::MorphTracksComponent;
    }else if(normalizedName == "panel"){
        return ComponentType::PanelComponent;
    }else if(normalizedName == "particles"){
        return ComponentType::ParticlesComponent;
    }else if(normalizedName == "points"){
        return ComponentType::PointsComponent;
    }else if(normalizedName == "polygon"){
        return ComponentType::PolygonComponent;
    }else if(normalizedName == "positionaction"){
        return ComponentType::PositionActionComponent;
    }else if(normalizedName == "rotatetracks"){
        return ComponentType::RotateTracksComponent;
    }else if(normalizedName == "rotationaction"){
        return ComponentType::RotationActionComponent;
    }else if(normalizedName == "scaleaction"){
        return ComponentType::ScaleActionComponent;
    }else if(normalizedName == "scaletracks"){
        return ComponentType::ScaleTracksComponent;
    }else if(normalizedName == "script"){
        return ComponentType::ScriptComponent;
    }else if(normalizedName == "scrollbar"){
        return ComponentType::ScrollbarComponent;
    }else if(normalizedName == "sky"){
        return ComponentType::SkyComponent;
    }else if(normalizedName == "spriteanimation"){
        return ComponentType::SpriteAnimationComponent;
    }else if(normalizedName == "sprite"){
        return ComponentType::SpriteComponent;
    }else if(normalizedName == "terrain"){
        return ComponentType::TerrainComponent;
    }else if(normalizedName == "text"){
        return ComponentType::TextComponent;
    }else if(normalizedName == "textedit"){
        return ComponentType::TextEditComponent;
    }else if(normalizedName == "tilemap"){
        return ComponentType::TilemapComponent;
    }else if(normalizedName == "timedaction"){
        return ComponentType::TimedActionComponent;
    }else if(normalizedName == "transform"){
        return ComponentType::Transform;
    }else if(normalizedName == "translatetracks"){
        return ComponentType::TranslateTracksComponent;
    }else if(normalizedName == "ui"){
        return ComponentType::UIComponent;
    }else if(normalizedName == "uicontainer"){
        return ComponentType::UIContainerComponent;
    }else if(normalizedName == "uilayout"){
        return ComponentType::UILayoutComponent;
    }

    throw std::invalid_argument("Unknown component type: " + componentName);
}

Signature Editor::Catalog::componentTypeToSignature(const EntityRegistry* registry, ComponentType compType) {
    Signature signature;
    ComponentId cid = getComponentId(registry, compType);
    signature.set(cid, true);
    return signature;
}

Signature Editor::Catalog::componentMaskToSignature(const EntityRegistry* registry, uint64_t mask) {
    Signature signature;
    for (int i = 0; i < 64; ++i) {
        if ((mask >> i) & 1) {
            auto compType = static_cast<ComponentType>(i);
            ComponentId cid = getComponentId(registry, compType);
            signature.set(cid, true);
        }
    }
    return signature;
}

Editor::PropertyType Editor::Catalog::scriptPropertyTypeToPropertyType(ScriptPropertyType scriptType) {
    switch (scriptType) {
        case Supernova::ScriptPropertyType::Bool: return Editor::PropertyType::Bool;
        case Supernova::ScriptPropertyType::Int: return Editor::PropertyType::Int;
        case Supernova::ScriptPropertyType::Float: return Editor::PropertyType::Float;
        case Supernova::ScriptPropertyType::String: return Editor::PropertyType::String;
        case Supernova::ScriptPropertyType::Vector2: return Editor::PropertyType::Vector2;
        case Supernova::ScriptPropertyType::Vector3: return Editor::PropertyType::Vector3;
        case Supernova::ScriptPropertyType::Vector4: return Editor::PropertyType::Vector4;
        case Supernova::ScriptPropertyType::Color3: return Editor::PropertyType::Color3L;
        case Supernova::ScriptPropertyType::Color4: return Editor::PropertyType::Color4L;
        case Supernova::ScriptPropertyType::Pointer: return Editor::PropertyType::Custom;
        default: return Editor::PropertyType::Custom;
    }
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
        ps["fakeBillboard"] = {PropertyType::Bool, UpdateFlags_Transform, (void*)&def->fakeBillboard, (compRef) ? (void*)&comp->fakeBillboard : nullptr};
        ps["cylindricalBillboard"] = {PropertyType::Bool, UpdateFlags_Transform, (void*)&def->cylindricalBillboard, (compRef) ? (void*)&comp->cylindricalBillboard : nullptr};
        ps["billboardRotation"] = {PropertyType::Quat, UpdateFlags_Transform, (void*)&def->billboardRotation, (compRef) ? (void*)&comp->billboardRotation : nullptr};

    }else if (component == ComponentType::MeshComponent){
        MeshComponent* comp = (MeshComponent*)compRef;
        static MeshComponent* def = new MeshComponent;

        ps["castShadows"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->castShadows, (compRef) ? (void*)&comp->castShadows : nullptr};
        ps["receiveShadows"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->receiveShadows, (compRef) ? (void*)&comp->receiveShadows : nullptr};
        ps["numSubmeshes"] = {PropertyType::UInt, UpdateFlags_None, (void*)&def->numSubmeshes, (compRef) ? (void*)&comp->numSubmeshes : nullptr};
        for (int s = 0; s < ((compRef) ? MAX_SUBMESHES : 1); s++){
            std::string idx = (compRef) ? std::to_string(s) : "";
            ps["submeshes["+idx+"].material"] = {PropertyType::Material, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material, (compRef) ? (void*)&comp->submeshes[s].material : nullptr};
            ps["submeshes["+idx+"].material.name"] = {PropertyType::String, UpdateFlags_None, (void*)&def->submeshes[0].material.name, (compRef) ? (void*)&comp->submeshes[s].material.name : nullptr};
            ps["submeshes["+idx+"].material.baseColor"] = {PropertyType::Color4L, UpdateFlags_None, (void*)&def->submeshes[0].material.baseColorFactor, (compRef) ? (void*)&comp->submeshes[s].material.baseColorFactor : nullptr};
            ps["submeshes["+idx+"].material.metallicFactor"] = {PropertyType::Float_0_1, UpdateFlags_None, (void*)&def->submeshes[0].material.metallicFactor, (compRef) ? (void*)&comp->submeshes[s].material.metallicFactor : nullptr};
            ps["submeshes["+idx+"].material.roughnessFactor"] = {PropertyType::Float_0_1, UpdateFlags_None, (void*)&def->submeshes[0].material.roughnessFactor, (compRef) ? (void*)&comp->submeshes[s].material.roughnessFactor : nullptr};
            ps["submeshes["+idx+"].material.emissiveFactor"] = {PropertyType::Color3L, UpdateFlags_None, (void*)&def->submeshes[0].material.emissiveFactor, (compRef) ? (void*)&comp->submeshes[s].material.emissiveFactor : nullptr};
            ps["submeshes["+idx+"].material.baseColorTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.baseColorTexture, (compRef) ? (void*)&comp->submeshes[s].material.baseColorTexture : nullptr};
            ps["submeshes["+idx+"].material.emissiveTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.emissiveTexture, (compRef) ? (void*)&comp->submeshes[s].material.emissiveTexture : nullptr};
            ps["submeshes["+idx+"].material.metallicRoughnessTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.metallicRoughnessTexture, (compRef) ? (void*)&comp->submeshes[s].material.metallicRoughnessTexture : nullptr};
            ps["submeshes["+idx+"].material.occlusionTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.occlusionTexture, (compRef) ? (void*)&comp->submeshes[s].material.occlusionTexture : nullptr};
            ps["submeshes["+idx+"].material.normalTexture"] = {PropertyType::Texture, UpdateFlags_Mesh_Texture, (void*)&def->submeshes[0].material.normalTexture, (compRef) ? (void*)&comp->submeshes[s].material.normalTexture : nullptr};

            ps["submeshes["+idx+"].primitiveType"] = {PropertyType::Enum, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].primitiveType, (compRef) ? (void*)&comp->submeshes[s].primitiveType : nullptr, &entriesPrimitiveType};
            ps["submeshes["+idx+"].faceCulling"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].faceCulling, (compRef) ? (void*)&comp->submeshes[s].faceCulling : nullptr};
            ps["submeshes["+idx+"].textureShadow"] = {PropertyType::Bool, UpdateFlags_Mesh_Reload, (void*)&def->submeshes[0].textureShadow, (compRef) ? (void*)&comp->submeshes[s].textureShadow : nullptr};
            ps["submeshes["+idx+"].textureRect"] = {PropertyType::Vector4, UpdateFlags_None, (void*)&def->submeshes[0].textureRect, (compRef) ? (void*)&comp->submeshes[s].textureRect : nullptr};
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
        ps["ignoreScissor"] = {PropertyType::Bool, UpdateFlags_None, (void*)&def->ignoreScissor, (compRef) ? (void*)&comp->ignoreScissor : nullptr};
    }else if (component == ComponentType::ImageComponent){
        ImageComponent* comp = (ImageComponent*)compRef;
        static ImageComponent* def = new ImageComponent;

        ps["patchMarginLeft"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginLeft, (compRef) ? (void*)&comp->patchMarginLeft : nullptr};
        ps["patchMarginRight"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginRight, (compRef) ? (void*)&comp->patchMarginRight : nullptr};
        ps["patchMarginTop"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginTop, (compRef) ? (void*)&comp->patchMarginTop : nullptr};
        ps["patchMarginBottom"] = {PropertyType::UInt, UpdateFlags_Image_Patches, (void*)&def->patchMarginBottom, (compRef) ? (void*)&comp->patchMarginBottom : nullptr};
        ps["textureScaleFactor"] = {PropertyType::Float, UpdateFlags_Image_Patches, (void*)&def->textureScaleFactor, (compRef) ? (void*)&comp->textureScaleFactor : nullptr};
    }else if (component == ComponentType::SpriteComponent){
        SpriteComponent* comp = (SpriteComponent*)compRef;
        static SpriteComponent* def = new SpriteComponent;

        ps["width"] = {PropertyType::UInt, UpdateFlags_Sprite, nullptr, (compRef) ? (void*)&comp->width : nullptr};
        ps["height"] = {PropertyType::UInt, UpdateFlags_Sprite, nullptr, (compRef) ? (void*)&comp->height : nullptr};
        ps["pivotPreset"] = {PropertyType::Enum, UpdateFlags_Sprite, (void*)&def->pivotPreset, (compRef) ? (void*)&comp->pivotPreset : nullptr, &entriesPivotPreset};
        ps["textureScaleFactor"] = {PropertyType::Float, UpdateFlags_Sprite, (void*)&def->textureScaleFactor, (compRef) ? (void*)&comp->textureScaleFactor : nullptr};
    }else if (component == ComponentType::LightComponent){
        LightComponent* comp = (LightComponent*)compRef;
        static LightComponent* def = new LightComponent;

        ps["type"] = {PropertyType::Enum, UpdateFlags_LightShadowMap | UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, nullptr, (compRef) ? (void*)&comp->type : nullptr, &entriesLightType};
        ps["direction"] = {PropertyType::Direction, UpdateFlags_Transform, (void*)&def->direction, (compRef) ? (void*)&comp->direction : nullptr};
        ps["shadows"] = {PropertyType::Bool, UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, (void*)&def->shadows, (compRef) ? (void*)&comp->shadows : nullptr};
        ps["intensity"] = {PropertyType::Float, UpdateFlags_None, (void*)&def->intensity, (compRef) ? (void*)&comp->intensity : nullptr};
        ps["range"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, (void*)&def->range, (compRef) ? (void*)&comp->range : nullptr};
        ps["color"] = {PropertyType::Color3L, UpdateFlags_None, (void*)&def->color, (compRef) ? (void*)&comp->color : nullptr};
        ps["innerConeCos"] = {PropertyType::HalfCone, UpdateFlags_LightShadowCamera, (void*)&def->innerConeCos, (compRef) ? (void*)&comp->innerConeCos : nullptr};
        ps["outerConeCos"] = {PropertyType::HalfCone, UpdateFlags_LightShadowCamera, (void*)&def->outerConeCos, (compRef) ? (void*)&comp->outerConeCos : nullptr};
        ps["shadowBias"] = {PropertyType::Float, UpdateFlags_None, (void*)&def->shadowBias, (compRef) ? (void*)&comp->shadowBias : nullptr};
        ps["mapResolution"] = {PropertyType::UIntSlider, UpdateFlags_LightShadowMap | UpdateFlags_Scene_Mesh_Reload, (void*)&def->mapResolution, (compRef) ? (void*)&comp->mapResolution : nullptr, nullptr, &po2Values};
        ps["automaticShadowCamera"] = {PropertyType::Bool, UpdateFlags_LightShadowCamera, (void*)&def->automaticShadowCamera, (compRef) ? (void*)&comp->automaticShadowCamera : nullptr};
        ps["shadowCameraNear"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, nullptr, (compRef) ? (void*)&comp->shadowCameraNearFar.x : nullptr};
        ps["shadowCameraFar"] = {PropertyType::Float, UpdateFlags_LightShadowCamera, nullptr, (compRef) ? (void*)&comp->shadowCameraNearFar.y : nullptr};
        ps["numShadowCascades"] = {PropertyType::UIntSlider, UpdateFlags_LightShadowCamera | UpdateFlags_Scene_Mesh_Reload, (void*)&def->numShadowCascades, (compRef) ? (void*)&comp->numShadowCascades : nullptr, nullptr, &cascadeValues};
    }else if (component == ComponentType::ScriptComponent){
        ScriptComponent* comp = (ScriptComponent*)compRef;
        static ScriptComponent* def = new ScriptComponent;

        ps["scripts"] = {PropertyType::Custom, UpdateFlags_None, nullptr, (compRef) ? (void*)&comp->scripts : nullptr};

        // Add script properties to the map
        if (compRef && comp) {
            for (size_t i = 0; i < comp->scripts.size(); i++) {
                auto& script = comp->scripts[i];
                if (!script.enabled) continue;

                for (auto& prop : script.properties) {
                    std::string key = "script[" + std::to_string(i) + "]." + prop.name;
                    PropertyData propData;
                    propData.type = scriptPropertyTypeToPropertyType(prop.type);
                    propData.updateFlags = UpdateFlags_None;

                    // Get pointer to the actual value in the variant
                    switch (prop.type) {
                        case Supernova::ScriptPropertyType::Bool:
                            propData.ref = &std::get<bool>(prop.value);
                            propData.def = &std::get<bool>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Int:
                            propData.ref = &std::get<int>(prop.value);
                            propData.def = &std::get<int>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Float:
                            propData.ref = &std::get<float>(prop.value);
                            propData.def = &std::get<float>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::String:
                            propData.ref = &std::get<std::string>(prop.value);
                            propData.def = &std::get<std::string>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Vector2:
                            propData.ref = &std::get<Vector2>(prop.value);
                            propData.def = &std::get<Vector2>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Vector3:
                        case Supernova::ScriptPropertyType::Color3:
                            propData.ref = &std::get<Vector3>(prop.value);
                            propData.def = &std::get<Vector3>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Vector4:
                        case Supernova::ScriptPropertyType::Color4:
                            propData.ref = &std::get<Vector4>(prop.value);
                            propData.def = &std::get<Vector4>(prop.defaultValue);
                            break;
                        case Supernova::ScriptPropertyType::Pointer:
                            propData.ref = &std::get<void*>(prop.value);
                            propData.def = &std::get<void*>(prop.defaultValue);
                            break;
                        default:
                            propData.ref = nullptr;
                            propData.def = nullptr;
                            break;
                    }

                    ps[key] = propData;
                }
            }
        }
    }

    return ps;
}

std::vector<Editor::ComponentType> Editor::Catalog::findComponents(EntityRegistry* registry, Entity entity){
    std::vector<Editor::ComponentType> ret;

    if (registry->findComponent<ActionComponent>(entity)){
        ret.push_back(ComponentType::ActionComponent);
    }
    if (registry->findComponent<AlphaActionComponent>(entity)){
        ret.push_back(ComponentType::AlphaActionComponent);
    }
    if (registry->findComponent<AudioComponent>(entity)){
        ret.push_back(ComponentType::AudioComponent);
    }
    if (registry->findComponent<Body2DComponent>(entity)){
        ret.push_back(ComponentType::Body2DComponent);
    }
    if (registry->findComponent<Body3DComponent>(entity)){
        ret.push_back(ComponentType::Body3DComponent);
    }
    if (registry->findComponent<BoneComponent>(entity)){
        ret.push_back(ComponentType::BoneComponent);
    }
    if (registry->findComponent<ButtonComponent>(entity)){
        ret.push_back(ComponentType::ButtonComponent);
    }
    if (registry->findComponent<CameraComponent>(entity)){
        ret.push_back(ComponentType::CameraComponent);
    }
    if (registry->findComponent<ColorActionComponent>(entity)){
        ret.push_back(ComponentType::ColorActionComponent);
    }
    if (registry->findComponent<FogComponent>(entity)){
        ret.push_back(ComponentType::FogComponent);
    }
    if (registry->findComponent<ImageComponent>(entity)){
        ret.push_back(ComponentType::ImageComponent);
    }
    if (registry->findComponent<InstancedMeshComponent>(entity)){
        ret.push_back(ComponentType::InstancedMeshComponent);
    }
    if (registry->findComponent<Joint2DComponent>(entity)){
        ret.push_back(ComponentType::Joint2DComponent);
    }
    if (registry->findComponent<Joint3DComponent>(entity)){
        ret.push_back(ComponentType::Joint3DComponent);
    }
    if (registry->findComponent<KeyframeTracksComponent>(entity)){
        ret.push_back(ComponentType::KeyframeTracksComponent);
    }
    if (registry->findComponent<LightComponent>(entity)){
        ret.push_back(ComponentType::LightComponent);
    }
    if (registry->findComponent<LinesComponent>(entity)){
        ret.push_back(ComponentType::LinesComponent);
    }
    if (registry->findComponent<MeshComponent>(entity)){
        ret.push_back(ComponentType::MeshComponent);
    }
    if (registry->findComponent<MeshPolygonComponent>(entity)){
        ret.push_back(ComponentType::MeshPolygonComponent);
    }
    if (registry->findComponent<ModelComponent>(entity)){
        ret.push_back(ComponentType::ModelComponent);
    }
    if (registry->findComponent<MorphTracksComponent>(entity)){
        ret.push_back(ComponentType::MorphTracksComponent);
    }
    if (registry->findComponent<PanelComponent>(entity)){
        ret.push_back(ComponentType::PanelComponent);
    }
    if (registry->findComponent<ParticlesComponent>(entity)){
        ret.push_back(ComponentType::ParticlesComponent);
    }
    if (registry->findComponent<PointsComponent>(entity)){
        ret.push_back(ComponentType::PointsComponent);
    }
    if (registry->findComponent<PolygonComponent>(entity)){
        ret.push_back(ComponentType::PolygonComponent);
    }
    if (registry->findComponent<PositionActionComponent>(entity)){
        ret.push_back(ComponentType::PositionActionComponent);
    }
    if (registry->findComponent<RotateTracksComponent>(entity)){
        ret.push_back(ComponentType::RotateTracksComponent);
    }
    if (registry->findComponent<RotationActionComponent>(entity)){
        ret.push_back(ComponentType::RotationActionComponent);
    }
    if (registry->findComponent<ScaleActionComponent>(entity)){
        ret.push_back(ComponentType::ScaleActionComponent);
    }
    if (registry->findComponent<ScaleTracksComponent>(entity)){
        ret.push_back(ComponentType::ScaleTracksComponent);
    }
    if (registry->findComponent<ScriptComponent>(entity)){
        ret.push_back(ComponentType::ScriptComponent);
    }
    if (registry->findComponent<ScrollbarComponent>(entity)){
        ret.push_back(ComponentType::ScrollbarComponent);
    }
    if (registry->findComponent<SkyComponent>(entity)){
        ret.push_back(ComponentType::SkyComponent);
    }
    if (registry->findComponent<SpriteAnimationComponent>(entity)){
        ret.push_back(ComponentType::SpriteAnimationComponent);
    }
    if (registry->findComponent<SpriteComponent>(entity)){
        ret.push_back(ComponentType::SpriteComponent);
    }
    if (registry->findComponent<TerrainComponent>(entity)){
        ret.push_back(ComponentType::TerrainComponent);
    }
    if (registry->findComponent<TextComponent>(entity)){
        ret.push_back(ComponentType::TextComponent);
    }
    if (registry->findComponent<TextEditComponent>(entity)){
        ret.push_back(ComponentType::TextEditComponent);
    }
    if (registry->findComponent<TilemapComponent>(entity)){
        ret.push_back(ComponentType::TilemapComponent);
    }
    if (registry->findComponent<TimedActionComponent>(entity)){
        ret.push_back(ComponentType::TimedActionComponent);
    }
    if (registry->findComponent<Transform>(entity)){
        ret.push_back(ComponentType::Transform);
    }
    if (registry->findComponent<TranslateTracksComponent>(entity)){
        ret.push_back(ComponentType::TranslateTracksComponent);
    }
    if (registry->findComponent<UIComponent>(entity)){
        ret.push_back(ComponentType::UIComponent);
    }
    if (registry->findComponent<UIContainerComponent>(entity)){
        ret.push_back(ComponentType::UIContainerComponent);
    }
    if (registry->findComponent<UILayoutComponent>(entity)){
        ret.push_back(ComponentType::UILayoutComponent);
    }

    return ret;
}

std::map<std::string, Editor::PropertyData> Editor::Catalog::findEntityProperties(EntityRegistry* registry, Entity entity, ComponentType component){
    if(component == ComponentType::Transform){
        if (Transform* compRef = registry->findComponent<Transform>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::MeshComponent){
        if (MeshComponent* compRef = registry->findComponent<MeshComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::UIComponent){
        if (UIComponent* compRef = registry->findComponent<UIComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::UILayoutComponent){
        if (UILayoutComponent* compRef = registry->findComponent<UILayoutComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::ImageComponent){
        if (ImageComponent* compRef = registry->findComponent<ImageComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::SpriteComponent){
        if (SpriteComponent* compRef = registry->findComponent<SpriteComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::LightComponent){
        if (LightComponent* compRef = registry->findComponent<LightComponent>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::ScriptComponent){
        if (ScriptComponent* compRef = registry->findComponent<ScriptComponent>(entity)){
            return getProperties(component, compRef);
        }
    }

    return std::map<std::string, Editor::PropertyData>();
}

void Editor::Catalog::updateEntity(EntityRegistry* registry, Entity entity, int updateFlags){
    if (updateFlags & UpdateFlags_Transform){
        registry->getComponent<Transform>(entity).needUpdate = true;
    }
    if (updateFlags & UpdateFlags_Scene_Mesh_Reload){
        auto meshes = registry->getComponentArray<MeshComponent>();
        for (int i = 0; i < meshes->size(); i++) {
            MeshComponent& mesh = meshes->getComponentFromIndex(i);
            mesh.needReload = true;
        }
    }
    if (updateFlags & UpdateFlags_Mesh_Reload){
        registry->getComponent<MeshComponent>(entity).needReload = true;
    }
    if (updateFlags & UpdateFlags_Mesh_Texture){
        unsigned int numSubmeshes = registry->getComponent<MeshComponent>(entity).numSubmeshes;
        for (unsigned int i = 0; i < numSubmeshes; i++){
            registry->getComponent<MeshComponent>(entity).submeshes[i].needUpdateTexture = true;
        }
    }
    if (updateFlags & UpdateFlags_LightShadowMap){
        registry->getComponent<LightComponent>(entity).needUpdateShadowMap = true;
    }
    if (updateFlags & UpdateFlags_LightShadowCamera){
        registry->getComponent<LightComponent>(entity).needUpdateShadowCamera = true;
    }
    if (updateFlags & UpdateFlags_UI_Reload){
        registry->getComponent<UIComponent>(entity).needReload = true;
    }
    if (updateFlags & UpdateFlags_UI_Texture){
        registry->getComponent<UIComponent>(entity).needUpdateTexture = true;
    }
    if (updateFlags & UpdateFlags_Image_Patches){
        registry->getComponent<ImageComponent>(entity).needUpdatePatches = true;
    }
    if (updateFlags & UpdateFlags_Layout_Sizes){
        registry->getComponent<UILayoutComponent>(entity).needUpdateSizes = true;
    }
    if (updateFlags & UpdateFlags_Sprite){
        registry->getComponent<SpriteComponent>(entity).needUpdateSprite = true;
    }
}

void Editor::Catalog::copyComponent(EntityRegistry* sourceRegistry, Entity sourceEntity,
                                   EntityRegistry* targetRegistry, Entity targetEntity,
                                   ComponentType compType) {

    switch (compType) {
        case ComponentType::Transform: {
            Entity parent = targetRegistry->getComponent<Transform>(targetEntity).parent;
            YAML::Node encoded = Stream::encodeTransform(sourceRegistry->getComponent<Transform>(sourceEntity));
            targetRegistry->getComponent<Transform>(targetEntity) = Stream::decodeTransform(encoded);
            targetRegistry->getComponent<Transform>(targetEntity).parent = parent; // not need to re-order because it is same parent
            break;
        }

        case ComponentType::MeshComponent: {
            YAML::Node encoded = Stream::encodeMeshComponent(sourceRegistry->getComponent<MeshComponent>(sourceEntity));
            targetRegistry->getComponent<MeshComponent>(targetEntity) = Stream::decodeMeshComponent(encoded);
            break;
        }

        case ComponentType::UIComponent: {
            YAML::Node encoded = Stream::encodeUIComponent(sourceRegistry->getComponent<UIComponent>(sourceEntity));
            targetRegistry->getComponent<UIComponent>(targetEntity) = Stream::decodeUIComponent(encoded);
            break;
        }

        case ComponentType::UILayoutComponent: {
            YAML::Node encoded = Stream::encodeUILayoutComponent(sourceRegistry->getComponent<UILayoutComponent>(sourceEntity));
            targetRegistry->getComponent<UILayoutComponent>(targetEntity) = Stream::decodeUILayoutComponent(encoded);
            break;
        }

        case ComponentType::ImageComponent: {
            YAML::Node encoded = Stream::encodeImageComponent(sourceRegistry->getComponent<ImageComponent>(sourceEntity));
            targetRegistry->getComponent<ImageComponent>(targetEntity) = Stream::decodeImageComponent(encoded);
            break;
        }

        case ComponentType::SpriteComponent: {
            YAML::Node encoded = Stream::encodeSpriteComponent(sourceRegistry->getComponent<SpriteComponent>(sourceEntity));
            targetRegistry->getComponent<SpriteComponent>(targetEntity) = Stream::decodeSpriteComponent(encoded);
            break;
        }

        case ComponentType::LightComponent: {
            YAML::Node encoded = Stream::encodeLightComponent(sourceRegistry->getComponent<LightComponent>(sourceEntity));
            targetRegistry->getComponent<LightComponent>(targetEntity) = Stream::decodeLightComponent(encoded);
            break;
        }

        case ComponentType::ScriptComponent: {
            YAML::Node encoded = Stream::encodeScriptComponent(sourceRegistry->getComponent<ScriptComponent>(sourceEntity));
            targetRegistry->getComponent<ScriptComponent>(targetEntity) = Stream::decodeScriptComponent(encoded);
            break;
        }

        default:
            printf("WARNING: Unsupported component type for copying: %s\n", getComponentName(compType).c_str());
            break;
    }
}

void Editor::Catalog::copyPropertyValue(EntityRegistry* sourceRegistry, Entity sourceEntity, 
                                       EntityRegistry* targetRegistry, Entity targetEntity, 
                                       ComponentType compType, const std::string& property) {

    // Get the property data to determine the type
    auto sourceProperties = Catalog::findEntityProperties(sourceRegistry, sourceEntity, compType);
    auto propIt = sourceProperties.find(property);
    if (propIt == sourceProperties.end()) {
        return; // Property not found
    }

    PropertyType propType = propIt->second.type;

    // Copy based on property type
    switch (propType) {
        case PropertyType::Bool: {
            bool* source = Catalog::getPropertyRef<bool>(sourceRegistry, sourceEntity, compType, property);
            bool* target = Catalog::getPropertyRef<bool>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Float:
        case PropertyType::Float_0_1: {
            float* source = Catalog::getPropertyRef<float>(sourceRegistry, sourceEntity, compType, property);
            float* target = Catalog::getPropertyRef<float>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Int: {
            int* source = Catalog::getPropertyRef<int>(sourceRegistry, sourceEntity, compType, property);
            int* target = Catalog::getPropertyRef<int>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::UInt:
        case PropertyType::UIntSlider: {
            unsigned int* source = Catalog::getPropertyRef<unsigned int>(sourceRegistry, sourceEntity, compType, property);
            unsigned int* target = Catalog::getPropertyRef<unsigned int>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector2: {
            Vector2* source = Catalog::getPropertyRef<Vector2>(sourceRegistry, sourceEntity, compType, property);
            Vector2* target = Catalog::getPropertyRef<Vector2>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector3:
        case PropertyType::Color3L:
        case PropertyType::Direction: {
            Vector3* source = Catalog::getPropertyRef<Vector3>(sourceRegistry, sourceEntity, compType, property);
            Vector3* target = Catalog::getPropertyRef<Vector3>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Vector4:
        case PropertyType::Color4L: {
            Vector4* source = Catalog::getPropertyRef<Vector4>(sourceRegistry, sourceEntity, compType, property);
            Vector4* target = Catalog::getPropertyRef<Vector4>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Quat: {
            Quaternion* source = Catalog::getPropertyRef<Quaternion>(sourceRegistry, sourceEntity, compType, property);
            Quaternion* target = Catalog::getPropertyRef<Quaternion>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::String: {
            std::string* source = Catalog::getPropertyRef<std::string>(sourceRegistry, sourceEntity, compType, property);
            std::string* target = Catalog::getPropertyRef<std::string>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Material: {
            Material* source = Catalog::getPropertyRef<Material>(sourceRegistry, sourceEntity, compType, property);
            Material* target = Catalog::getPropertyRef<Material>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Texture: {
            Texture* source = Catalog::getPropertyRef<Texture>(sourceRegistry, sourceEntity, compType, property);
            Texture* target = Catalog::getPropertyRef<Texture>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::HalfCone: {
            float* source = Catalog::getPropertyRef<float>(sourceRegistry, sourceEntity, compType, property);
            float* target = Catalog::getPropertyRef<float>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Enum: {
            int* source = Catalog::getPropertyRef<int>(sourceRegistry, sourceEntity, compType, property);
            int* target = Catalog::getPropertyRef<int>(targetRegistry, targetEntity, compType, property);
            if (source && target) *target = *source;
            break;
        }
        case PropertyType::Custom:
            // Custom types like scripts array are handled separately above
            break;
        default:
            // For any unknown/unsupported type, do nothing
            break;
    }

    // Apply any update flags that are associated with this property
    updateEntity(targetRegistry, targetEntity, propIt->second.updateFlags);
}

Editor::PropertyData Editor::Catalog::getProperty(EntityRegistry* registry, Entity entity, ComponentType component, std::string propertyName){
    for (auto& [name, property] : Catalog::findEntityProperties(registry, entity, component)){
        if (name == propertyName){
            return property;
        }
    }

    printf("ERROR: Cannot find property %s\n", propertyName.c_str());
    return PropertyData();
}