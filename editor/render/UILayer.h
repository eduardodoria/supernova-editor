#ifndef UILAYER_H
#define UILAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"
#include "RenderUtil.h"

namespace Supernova::Editor{

    class UILayer{
    private:
        Scene* scene;
        Camera* camera;

        Image* viewGizmoImage;

        CursorSelected cursorSelected;
    public:
        UILayer();

        void setViewportGizmoTexture(Framebuffer* framebuffer);

        void updateSize(int width, int height);

        void enableCursorPointer();
        void enableCursorHand();

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
        CursorSelected getCursorSelected();
    };

}

#endif /* UILAYER_H */