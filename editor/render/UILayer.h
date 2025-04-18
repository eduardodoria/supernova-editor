#ifndef UILAYER_H
#define UILAYER_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"
#include "object/ui/Image.h"
#include "object/ui/Polygon.h"

namespace Supernova::Editor{

    class UILayer{
    private:
        Scene* scene;
        Camera* camera;

        Object* rect;

        Polygon* upRect;
        Polygon* bottomRect;
        Polygon* leftRect;
        Polygon* rightRect;
        Polygon* centralRect;

        Image* viewGizmoImage;
    public:
        UILayer(bool enableViewGizmo = true);
        virtual ~UILayer();

        void setViewportGizmoTexture(Framebuffer* framebuffer);

        void setRectVisible(bool visible);
        void updateRect(Vector2 position, Vector2 size);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Camera* getCamera();
        Scene* getScene();
    };

}

#endif /* UILAYER_H */