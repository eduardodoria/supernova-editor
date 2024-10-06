#ifndef GIZMOS_H
#define GIZMOS_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

namespace Supernova::Editor{

    class Gizmos{
    private:
        Scene* scene;
        Camera* camera;

        Shape* gizmo;
        Image* gimbalImage;
    public:
        Gizmos();

        void setGimbalTexture(Framebuffer* framebuffer);

        void updateSize(int width, int height);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        Shape* getGizmo();
    };

}

#endif /* GIZMOS_H */