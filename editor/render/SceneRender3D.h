#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "ViewportGizmo.h"

namespace Supernova::Editor{

    struct LightObjects{
        Sprite* icon = nullptr;
        Lines* lines = nullptr;

        LightType type;
        float innerConeCos = 0.0f;
        float outerConeCos = 0.0f;
        Vector3 direction = Vector3::ZERO;
        float range = 0.0f;
    };

    struct CameraObjects{
        Sprite* icon = nullptr;
        Lines* lines = nullptr;

        CameraType type;
        float yfov = 0;
        float aspect = 0;
        float nearClip = 0;
        float farClip = 0;
        float leftClip = 0;
        float rightClip = 0;
        float bottomClip = 0;
        float topClip = 0;
    };

    class SceneRender3D: public SceneRender{
    private:

        Lines* lines;
        SkyBox* sky;

        std::map<Entity, LightObjects> lightObjects;
        std::map<Entity, CameraObjects> cameraObjects;

        ViewportGizmo viewgizmo;

        Vector2 linesOffset;

        void createLines();
        bool instanciateLightObject(Entity entity);
        bool instanciateCameraObject(Entity entity);
        void createOrUpdateLightIcon(Entity entity, const Transform& transform, LightType lightType, bool newLight);
        void createOrUpdateCameraIcon(Entity entity, const Transform& transform, bool newCamera);
        void createCameraFrustum(Entity entity, const Transform& transform, const CameraComponent& cameraComponent, bool isSelected);
        void createDirectionalLightArrow(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected);
        void createPointLightSphere(Entity entity, const Transform& transform, const LightComponent& light, bool isSelected);
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
        virtual void mouseDragEvent(float x, float y, float origX, float origY, Project* project, size_t sceneId, std::vector<Entity> selEntities, bool disableSelection);

        ViewportGizmo* getViewportGizmo();
    };

}