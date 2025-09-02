#pragma once

#include <stddef.h>
#include <string>
#include <vector>
#include "Scene.h"

namespace Supernova::Editor{
    enum UpdateFlags{
        UpdateFlags_None                = 0,
        UpdateFlags_Transform           = 1 << 0,
        UpdateFlags_Scene_Mesh_Reload   = 1 << 1,
        UpdateFlags_Mesh_Reload         = 1 << 2,
        UpdateFlags_Mesh_Texture        = 1 << 3,
        UpdateFlags_UI_Reload           = 1 << 4,
        UpdateFlags_UI_Texture          = 1 << 5,
        UpdateFlags_Image_Patches       = 1 << 6,
        UpdateFlags_Layout_Sizes        = 1 << 7,
        UpdateFlags_Sprite              = 1 << 8,
        UpdateFlags_LightShadowMap      = 1 << 9,
        UpdateFlags_LightShadowCamera   = 1 << 10
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
        HalfCone,
        UIntSlider,
        IntSlider,
        Direction,
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
        std::vector<int>* sliderValues = nullptr;
    };

    class Catalog{
    private:

        static std::string removeComponentSuffix(std::string str);

    public:
        Catalog();

        static std::string getComponentName(ComponentType component, bool removeSuffix = false);
        static ComponentId getComponentId(const EntityRegistry* registry, ComponentType compType);
        static ComponentType getComponentType(const std::string& componentName);
        static Signature componentTypeMaskToSignature(const EntityRegistry* registry, uint64_t mask);

        static std::map<std::string, PropertyData> getProperties(ComponentType component, void* compRef);

        static std::vector<ComponentType> findComponents(EntityRegistry* registry, Entity entity);
        static std::map<std::string, PropertyData> findEntityProperties(EntityRegistry* registry, Entity entity, ComponentType component);

        static void updateEntity(EntityRegistry* registry, Entity entity, int updateFlags);

        static void copyPropertyValue(EntityRegistry* sourceRegistry, Entity sourceEntity, 
                                    EntityRegistry* targetRegistry, Entity targetEntity, 
                                    ComponentType compType, const std::string& property);

        template<typename T>
        static T* getPropertyRef(EntityRegistry* registry, Entity entity, ComponentType component, std::string propertyName){
            for (auto& [name, property] : Catalog::findEntityProperties(registry, entity, component)){
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
                if constexpr (std::is_same_v<T, LightState>) return LightState::AUTO;
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
            else if (propertyName == "global_illumination_color") {
                if constexpr (std::is_same_v<T, Vector3>) {
                    return scene->getGlobalIlluminationColor();
                }
            }
            else if (propertyName == "global_illumination_intensity") {
                if constexpr (std::is_same_v<T, float>) {
                    return scene->getGlobalIlluminationIntensity();
                }
            }
            else if (propertyName == "light_state") {
                if constexpr (std::is_same_v<T, LightState>) {
                    return scene->getLightState();
                }
            }

            // Return default value if property not found
            if constexpr (std::is_same_v<T, Vector4>) return Vector4(0.0, 0.0, 0.0, 1.0);
            if constexpr (std::is_same_v<T, Vector3>) return Vector3(1.0, 1.0, 1.0);
            if constexpr (std::is_same_v<T, bool>) return false;
            if constexpr (std::is_same_v<T, float>) return 0.0f;
            if constexpr (std::is_same_v<T, LightState>) return LightState::AUTO;
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
            else if (propertyName == "global_illumination_color") {
                if constexpr (std::is_same_v<T, Vector3>) {
                    scene->setGlobalIllumination(value);
                }
            }
            else if (propertyName == "global_illumination_intensity") {
                if constexpr (std::is_same_v<T, float>) {
                    scene->setGlobalIllumination(value);
                }
            }
            else if (propertyName == "light_state") {
                if constexpr (std::is_same_v<T, LightState>) {
                    scene->setLightState(value);
                }
            }
        }
    };

}
