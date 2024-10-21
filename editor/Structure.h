#ifndef STRUCTURE_H
#define STRUCTURE_H

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
        std::string name;
        std::string refName;
        void* ref;
    };

    class Structure{
    private:

    public:
        Structure();

        static size_t getPropertiesSize(ComponentType component);
        static std::vector<PropertyData> getProperties(ComponentType component, void* compRef);
        static std::vector<PropertyData> getProperties(ComponentType component, Scene* scene, Entity entity);

    };

}

#endif /* STRUCTURE_H */