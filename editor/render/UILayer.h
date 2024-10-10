#ifndef UILAYER_H
#define UILAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"

namespace Supernova::Editor{

    class UILayer{
    private:
        Scene* scene;
        Camera* camera;

        Image* viewGizmoImage;
    public:
        UILayer();

        void setViewportGizmoTexture(Framebuffer* framebuffer);

        void updateSize(int width, int height);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
    };

}

#endif /* UILAYER_H */