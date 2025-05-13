#pragma once

#include <stddef.h>
#include <string>
#include <vector>
#include "Scene.h"

namespace Supernova::Editor{
    enum UpdateFlags{
        UpdateFlags_None            = 0,
        UpdateFlags_Transform       = 1 << 0,
        UpdateFlags_Mesh_Reload     = 1 << 1,
        UpdateFlags_Mesh_Texture    = 1 << 2,
        UpdateFlags_UI_Reload       = 1 << 3,
        UpdateFlags_UI_Texture      = 1 << 4,
        UpdateFlags_Image_Patches   = 1 << 5,
        UpdateFlags_Layout_Sizes    = 1 << 6,
        UpdateFlags_Sprite          = 1 << 7
    };

    // the order of components here affects properties window
    enum class ComponentType : int {
        Transform,
        MeshComponent,
        UIComponent,
        UILayoutComponent,
        ActionComponent,
        AlphaActionComponent,
        AnimationComponent,
        AudioComponent,
        Body2DComponent,
        Body3DComponent,
        BoneComponent,
        ButtonComponent,
        CameraComponent,
        ColorActionComponent,
        FogComponent,
        ImageComponent,
        InstancedMeshComponent,
        Joint2DComponent,
        Joint3DComponent,
        KeyframeTracksComponent,
        LightComponent,
        LinesComponent,
        MeshPolygonComponent,
        ModelComponent,
        MorphTracksComponent,
        PanelComponent,
        ParticlesComponent,
        PointsComponent,
        PolygonComponent,
        PositionActionComponent,
        RotateTracksComponent,
        RotationActionComponent,
        ScaleActionComponent,
        ScaleTracksComponent,
        ScrollbarComponent,
        SkyComponent,
        SpriteAnimationComponent,
        SpriteComponent,
        TerrainComponent,
        TextComponent,
        TextEditComponent,
        TilemapComponent,
        TimedActionComponent,
        TranslateTracksComponent,
        UIContainerComponent
    };

    enum class PropertyType{
        Bool,
        String,
        Float,
        Float_0_1,
        Vector2,
        Vector3,
        Vector4,
        Quat,
        Color3L,
        Color4L,
        Int,
        UInt,
        Material,
        Texture,
        Enum
    };

    struct EnumEntry {
        int value;
        const char* name;
    };

    struct PropertyData{
        PropertyType type;
        int updateFlags;
        void* def;
        void* ref;
        std::vector<EnumEntry>* enumEntries = nullptr;
    };

    class Catalog{
    private:

    public:
        Catalog();

        static std::string getComponentName(ComponentType component);

        static std::map<std::string, PropertyData> getProperties(ComponentType component, void* compRef);

        static std::vector<ComponentType> findComponents(Scene* scene, Entity entity);
        static std::map<std::string, PropertyData> findEntityProperties(Scene* scene, Entity entity, ComponentType component);

        static void updateEntity(Scene* scene, Entity entity, int updateFlags);

        template<typename T>
        static T* getPropertyRef(Scene* scene, Entity entity, ComponentType component, std::string propertyName){
            for (auto& [name, property] : Catalog::findEntityProperties(scene, entity, component)){
                if (name == propertyName){
                    return static_cast<T*>(property.ref);
                }
            }

            printf("ERROR: Cannot find property %s\n", propertyName.c_str());
            return nullptr;
        }

        template<typename T>
        static T getSceneProperty(Scene* scene, const std::string& propertyName) {
            if (!scene) {
                // Return default values if scene is null
                if constexpr (std::is_same_v<T, Vector4>) return Vector4(0.0, 0.0, 0.0, 1.0);
                if constexpr (std::is_same_v<T, Vector3>) return Vector3(1.0, 1.0, 1.0);
                if constexpr (std::is_same_v<T, bool>) return false;
                if constexpr (std::is_same_v<T, float>) return 0.0f;
                // Add other types as needed
            }

            if (propertyName == "background_color") {
                if constexpr (std::is_same_v<T, Vector4>) {
                    return scene->getBackgroundColor();
                }
            }
            else if (propertyName == "shadows_pcf") {
                if constexpr (std::is_same_v<T, bool>) {
                    return scene->isShadowsPCF();
                }
            }
            else if (propertyName == "ambient_light_enabled") {
                if constexpr (std::is_same_v<T, bool>) {
                    return scene->isSceneAmbientLightEnabled();
                }
            }
            else if (propertyName == "ambient_light_color") {
                if constexpr (std::is_same_v<T, Vector3>) {
                    return scene->getAmbientLightColor();
                }
            }
            else if (propertyName == "ambient_light_color_linear") {
                if constexpr (std::is_same_v<T, Vector3>) {
                    return scene->getAmbientLightColorLinear();
                }
            }
            else if (propertyName == "ambient_light_intensity") {
                if constexpr (std::is_same_v<T, float>) {
                    return scene->getAmbientLightIntensity();
                }
            }

            // Return default value if property not found
            if constexpr (std::is_same_v<T, Vector4>) return Vector4(0.0, 0.0, 0.0, 1.0);
            if constexpr (std::is_same_v<T, Vector3>) return Vector3(1.0, 1.0, 1.0);
            if constexpr (std::is_same_v<T, bool>) return false;
            if constexpr (std::is_same_v<T, float>) return 0.0f;
            // Add other types as needed
        }

        template<typename T>
        static void setSceneProperty(Scene* scene, const std::string& propertyName, const T& value) {
            if (!scene) return;

            if (propertyName == "background_color") {
                if constexpr (std::is_same_v<T, Vector4>) {
                    scene->setBackgroundColor(value);
                }
            }
            else if (propertyName == "shadows_pcf") {
                if constexpr (std::is_same_v<T, bool>) {
                    scene->setShadowsPCF(value);
                }
            }
            else if (propertyName == "ambient_light_enabled") {
                if constexpr (std::is_same_v<T, bool>) {
                    scene->setSceneAmbientLightEnabled(value);
                }
            }
            else if (propertyName == "ambient_light_color") {
                if constexpr (std::is_same_v<T, Vector3>) {
                    scene->setAmbientLight(value);
                }
            }
            else if (propertyName == "ambient_light_intensity") {
                if constexpr (std::is_same_v<T, float>) {
                    scene->setAmbientLight(value);
                }
            }
        }
    };

}
