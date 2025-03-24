#ifndef CATALOG_H
#define CATALOG_H

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
        UpdateFlags_Layout_Sizes    = 1 << 5
    };

    enum class ComponentType : int {
        Transform,
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
        TranslateTracksComponent,
        UIComponent,
        UIContainerComponent,
        UILayoutComponent
    };

    enum class PropertyType{
        Bool,
        String,
        Float_0_1,
        Vector2,
        Vector3,
        Vector4,
        Quat,
        Color3L,
        Color4L,
        Int,
        UInt,
        Texture,
        PrimitiveType
    };

    struct PropertyData{
        PropertyType type;
        std::string label;
        int updateFlags;
        void* def;
        void* ref;
    };

    class Catalog{
    private:

    public:
        Catalog();

        static std::string getComponentName(ComponentType component);

        static std::map<std::string, PropertyData> getProperties(ComponentType component, void* compRef);

        static std::vector<ComponentType> findComponents(Scene* scene, Entity entity);
        static std::map<std::string, PropertyData> findEntityProperties(Scene* scene, Entity entity, ComponentType component);


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


        static std::vector<const char*> getPrimitiveTypeArray();
        static size_t getPrimitiveTypeToIndex(PrimitiveType pt);
        static PrimitiveType getPrimitiveTypeFromIndex(size_t i);
    };

}

#endif /* CATALOG_H */