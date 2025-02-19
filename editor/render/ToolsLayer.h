#ifndef TOOLSLAYER_H
#define TOOLSLAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

#include "RenderUtil.h"
#include "gizmo/TranslateGizmo.h"
#include "gizmo/RotateGizmo.h"
#include "gizmo/ScaleGizmo.h"

namespace Supernova::Editor{

    class ToolsLayer{
    private:
        Scene* scene;
        Camera* camera;

        TranslateGizmo* tGizmo;
        RotateGizmo* rGizmo;
        ScaleGizmo* sGizmo;

        GizmoSelected gizmoSelected;
        GizmoSideSelected gizmoSideSelected;
    public:
        ToolsLayer();
        virtual ~ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);
        void updateGizmo(Camera* sceneCam, Vector3& position, Quaternion& rotation, float scale, Ray& mouseRay, bool mouseClicked);

        void mouseDrag(Vector3 point);
        void mouseRelease();

        void enableTranslateGizmo();
        void enableRotateGizmo();
        void enableScaleGizmo();

        void setGizmoVisible(bool visible);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();

        Object* getGizmoObject() const;
        Vector3 getGizmoPosition() const;
        Quaternion getGizmoRotation() const;

        GizmoSelected getGizmoSelected() const;
        GizmoSideSelected getGizmoSideSelected() const;
    };

}

#endif /* TOOLSLAYER_H */