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
#include "gizmo/Object2DGizmo.h"

namespace Supernova::Editor{

    class ToolsLayer{
    private:
        Scene* scene;
        Camera* camera;

        TranslateGizmo* tGizmo;
        RotateGizmo* rGizmo;
        ScaleGizmo* sGizmo;
        Object2DGizmo* oGizmo;

        GizmoSelected gizmoSelected;
        GizmoSideSelected gizmoSideSelected;
        Gizmo2DSideSelected gizmo2DSideSelected;

        float gizmoScale;
    public:
        ToolsLayer(bool use2DGizmos);
        virtual ~ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);
        void updateGizmo(Camera* sceneCam, Vector3& position, Quaternion& rotation, float scale, AABB aabb, Matrix4 objectMatrix, Ray& mouseRay, bool mouseClicked);

        void mouseDrag(Vector3 point);
        void mouseRelease();

        void enableTranslateGizmo();
        void enableRotateGizmo();
        void enableScaleGizmo();
        void enableObject2DGizmo();

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
        Gizmo2DSideSelected getGizmo2DSideSelected() const;
    };

}

#endif /* TOOLSLAYER_H */