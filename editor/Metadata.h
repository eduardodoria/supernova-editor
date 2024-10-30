#ifndef METADATA_H
#define METADATA_H

#include <stddef.h>
#include <string>
#include <vector>
#include "Scene.h"

namespace Supernova::Editor{
    enum class ComponentType : int {
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
        MeshComponent,
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
        Transform,
        TranslateTracksComponent,
        UIComponent,
        UIContainerComponent,
        UILayoutComponent
    };

    enum class PropertyType{
        Bool,
        String,
        Float,
        Float2,
        Float3,
        Float4,
        Int
    };

    struct PropertyData{
        PropertyType type;
        std::string label;
        std::string name;
        void* ref;
    };

    class Metadata{
    private:

    public:
        Metadata();

        static size_t getPropertiesSize(ComponentType component);
        static std::vector<PropertyData> getProperties(ComponentType component, void* compRef);
        static std::vector<PropertyData> getProperties(Scene* scene, Entity entity, ComponentType component);

        template<typename T>
        static T* getPropertyRef(Scene* scene, Entity entity, ComponentType component, std::string propertyName){
            for (auto& property : Metadata::getProperties(scene, entity, component)) {
                if (property.name == propertyName){
                    return static_cast<T*>(property.ref);
                }
            }

            printf("ERROR: Cannot find property %s\n", propertyName.c_str());
            return nullptr;
        }

    };

}

#endif /* METADATA_H */