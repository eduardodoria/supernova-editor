#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "ViewportGizmo.h"

namespace Supernova::Editor{

    struct LightObjects{
        Sprite* icon = nullptr;
        Lines* lines = nullptr;

        float innerConeCos = 0.0f;
        float outerConeCos = 0.0f;
        Vector3 direction = Vector3::ZERO;
        float range = 0.0f;
    };

    class SceneRender3D: public SceneRender{
    private:

        Lines* lines;
        Light* sun;
        SkyBox* sky;

        std::map<Entity, LightObjects> lightObjects;

        ViewportGizmo viewgizmo;

        Vector2 linesOffset;

        void createLines();
        bool instanciateLightObject(Entity entity);
        void createOrUpdateLightIcon(Entity entity, const Transform& transform, LightType lightType, bool newLight);
        void createSpotLightCones(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected);

    public:
        SceneRender3D(Scene* scene);
        virtual ~SceneRender3D();

        virtual void activate();
        virtual void updateSelLines(std::vector<OBB> obbs);

        virtual void update(std::vector<Entity> selEntities, std::vector<Entity> entities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection);

        Light* getSunLight();
        ViewportGizmo* getViewportGizmo();
    };

}