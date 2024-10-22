#include "Structure.h"

#include "Scene.h"

using namespace Supernova;

Editor::Structure::Structure(){
}

size_t Editor::Structure::getPropertiesSize(ComponentType component){
    if(component == ComponentType::ActionComponent){
        return 1;
    }else if(component == ComponentType::AlphaActionComponent){
        return 1;
    }else if(component == ComponentType::AnimationComponent){
        return 1;
    }else if(component == ComponentType::AudioComponent){
        return 1;
    }else if(component == ComponentType::Body2DComponent){
        return 1;
    }else if(component == ComponentType::Body3DComponent){
        return 1;
    }else if(component == ComponentType::BoneComponent){
        return 1;
    }else if(component == ComponentType::ButtonComponent){
        return 1;
    }else if(component == ComponentType::CameraComponent){
        return 1;
    }else if(component == ComponentType::ColorActionComponent){
        return 1;
    }else if(component == ComponentType::FogComponent){
        return 1;
    }else if(component == ComponentType::ImageComponent){
        return 1;
    }else if(component == ComponentType::InstancedMeshComponent){
        return 1;
    }else if(component == ComponentType::Joint2DComponent){
        return 1;
    }else if(component == ComponentType::Joint3DComponent){
        return 1;
    }else if(component == ComponentType::KeyframeTracksComponent){
        return 1;
    }else if(component == ComponentType::LightComponent){
        return 1;
    }else if(component == ComponentType::LinesComponent){
        return 1;
    }else if(component == ComponentType::MeshComponent){
        return 1;
    }else if(component == ComponentType::MeshPolygonComponent){
        return 1;
    }else if(component == ComponentType::ModelComponent){
        return 1;
    }else if(component == ComponentType::MorphTracksComponent){
        return 1;
    }else if(component == ComponentType::PanelComponent){
        return 1;
    }else if(component == ComponentType::ParticlesComponent){
        return 1;
    }else if(component == ComponentType::PointsComponent){
        return 1;
    }else if(component == ComponentType::PolygonComponent){
        return 1;
    }else if(component == ComponentType::PositionActionComponent){
        return 1;
    }else if(component == ComponentType::RotateTracksComponent){
        return 1;
    }else if(component == ComponentType::RotationActionComponent){
        return 1;
    }else if(component == ComponentType::ScaleActionComponent){
        return 1;
    }else if(component == ComponentType::ScaleTracksComponent){
        return 1;
    }else if(component == ComponentType::ScrollbarComponent){
        return 1;
    }else if(component == ComponentType::SkyComponent){
        return 1;
    }else if(component == ComponentType::SpriteAnimationComponent){
        return 1;
    }else if(component == ComponentType::SpriteComponent){
        return 1;
    }else if(component == ComponentType::TerrainComponent){
        return 1;
    }else if(component == ComponentType::TextComponent){
        return 1;
    }else if(component == ComponentType::TextEditComponent){
        return 1;
    }else if(component == ComponentType::TilemapComponent){
        return 1;
    }else if(component == ComponentType::TimedActionComponent){
        return 1;
    }else if(component == ComponentType::Transform){
        return 1;
    }else if(component == ComponentType::TranslateTracksComponent){
        return 1;
    }else if(component == ComponentType::UIComponent){
        return 1;
    }else if(component == ComponentType::UIContainerComponent){
        return 1;
    }else if(component == ComponentType::UILayoutComponent){
        return 1;
    }

    return 0;
}

std::vector<Editor::PropertyData> Editor::Structure::getProperties(ComponentType component, void* compRef){
    std::vector<PropertyData> ps;
    if(component == ComponentType::Transform){
        Transform* comp = (Transform*)compRef;

        ps.push_back({PropertyType::Float3, "Position", "position", (void*)&comp->position});
        ps.push_back({PropertyType::Float4, "Rotation", "rotation", (void*)&comp->rotation});
        ps.push_back({PropertyType::Float3, "Scale", "scale", (void*)&comp->scale});
    }else if (component == ComponentType::MeshComponent){
        MeshComponent* comp = (MeshComponent*)compRef;

        ps.push_back({PropertyType::Bool, "Cast shadows", "castShadows", (void*)&comp->castShadows});
        ps.push_back({PropertyType::Bool, "Receive shadows", "receiveShadows", (void*)&comp->receiveShadows});
        ps.push_back({PropertyType::Bool, "Transparent", "transparent", (void*)&comp->transparent});
    }

    return ps;
}

std::vector<Editor::PropertyData> Editor::Structure::getProperties(Scene* scene, Entity entity, ComponentType component){
    if(component == ComponentType::Transform){
        if (Transform* compRef = scene->findComponent<Transform>(entity)){
            return getProperties(component, compRef);
        }
    }else if (component == ComponentType::MeshComponent){
        if (MeshComponent* compRef = scene->findComponent<MeshComponent>(entity)){
            return getProperties(component, compRef);
        }
    }

    return std::vector<PropertyData>();
}