#ifndef METADATA_H
#define METADATA_H

#include <stddef.h>
#include <string>
#include <vector>
#include "Scene.h"

namespace Supernova::Editor{
    enum UpdateFlags{
        UpdateFlags_None                     = 0,
        UpdateFlags_Transform                = 1 << 0,
        UpdateFlags_MeshReload               = 1 << 1,
    };

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
        Quat,
        Int
    };

    struct PropertyData{
        PropertyType type;
        std::string label;
        std::string name;
        int updateFlags;
        void* ref;
        std::vector<PropertyData> childs;
    };

    class Metadata{
    private:

    public:
        Metadata();

        static std::string getComponentName(ComponentType component);

        static std::vector<PropertyData> getProperties(ComponentType component, void* compRef);

        static std::vector<ComponentType> findComponents(Scene* scene, Entity entity);
        static std::vector<PropertyData> findProperties(Scene* scene, Entity entity, ComponentType component);


        template<typename T>
        static T* getPropertyRef(Scene* scene, Entity entity, ComponentType component, std::string propertyName){
            for (auto& property : Metadata::findProperties(scene, entity, component)) {
                if (property.name == propertyName){
                    return static_cast<T*>(property.ref);
                }
                for (auto& childConfig : property.childs){
                    if (childConfig.name == propertyName){
                        return static_cast<T*>(childConfig.ref);
                    }
                }
            }

            printf("ERROR: Cannot find property %s\n", propertyName.c_str());
            return nullptr;
        }
    };

}

#endif /* METADATA_H */