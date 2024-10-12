#ifndef TOOLSLAYER_H
#define TOOLSLAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

#include "RenderUtil.h"
#include "gizmo/TranslateGizmo.h"

namespace Supernova::Editor{

    class ToolsLayer{
    private:
        Scene* scene;
        Camera* camera;

        TranslateGizmo* tGizmo;

        GizmoSideSelected gizmoSideSelected;
    public:
        ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);
        void updateGizmo(Vector3& position, float scale, Ray& mouseRay, bool mouseClicked);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        Object* getGizmo();
        GizmoSideSelected getGizmoSideSelected() const;
    };

}

#endif /* TOOLSLAYER_H */