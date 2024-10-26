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

namespace Supernova::Editor{

    class ToolsLayer{
    private:
        Scene* scene;
        Camera* camera;

        TranslateGizmo* tGizmo;
        RotateGizmo* rGizmo;

        GizmoSelected gizmoSelected;
        GizmoSideSelected gizmoSideSelected;
    public:
        ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);
        void updateGizmo(Vector3& position, float scale, Ray& mouseRay, bool mouseClicked);

        void enableTranslateGizmo();
        void enableRotateGizmo();
        void enableScaleGizmo();

        void setGizmoVisible(bool visible);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        GizmoSelected getGizmoSelected() const;
        GizmoSideSelected getGizmoSideSelected() const;
    };

}

#endif /* TOOLSLAYER_H */