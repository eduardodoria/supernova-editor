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

        Image* gimbalImage;
    public:
        ToolsLayer();

        void setGimbalTexture(Framebuffer* framebuffer);

        void updateSize(int width, int height);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        Object* getGizmo();
    };

}

#endif /* TOOLSLAYER_H */