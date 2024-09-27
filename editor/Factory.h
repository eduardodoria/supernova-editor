#ifndef FACTORY_H
#define FACTORY_H

#include <stddef.h>
#include <string>
#include <vector>

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

    class Factory{
    private:

    public:
        Factory();

        size_t getPropertiesSize(ComponentType component);
        std::vector<PropertyData> getProperties(ComponentType component, void* compRef);

    };

}

#endif /* FACTORY_H */