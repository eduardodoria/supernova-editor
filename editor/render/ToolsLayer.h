#ifndef TOOLSLAYER_H
#define TOOLSLAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

namespace Supernova::Editor{

    class ToolsLayer{
    private:
        Scene* scene;
        Camera* camera;

        Object* gizmo;

        Shape* sphere;
        Shape* xaxis;
        Shape* yaxis;
        Shape* zaxis;
        Shape* xarrow;
        Shape* yarrow;
        Shape* zarrow;
    public:
        ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        Object* getGizmo();
    };

}

#endif /* TOOLSLAYER_H */