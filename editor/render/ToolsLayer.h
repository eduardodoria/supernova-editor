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

        static const Vector4 sphereColor;
        static const Vector4 xaxisColor;
        static const Vector4 yaxisColor;
        static const Vector4 zaxisColor;
        static const Vector4 sphereColorHightlight;
        static const Vector4 xaxisColorHightlight;
        static const Vector4 yaxisColorHightlight;
        static const Vector4 zaxisColorHightlight;

        bool checkHoverHighlight(Ray& ray);
    public:
        ToolsLayer();

        void updateCamera(CameraComponent& extCamera, Transform& extCameraTransform);
        bool updateGizmo(Vector3& position, float scale, Ray& mouseRay);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        Object* getGizmo();
    };

}

#endif /* TOOLSLAYER_H */