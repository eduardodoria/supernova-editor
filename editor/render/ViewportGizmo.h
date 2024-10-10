#ifndef VIEWPORTGIZMO_H
#define VIEWPORTGIZMO_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"

namespace Supernova::Editor{

    class ViewportGizmo{
    private:
        Scene* scene;
        Camera* camera;

        Object* mainObject;

        Shape* cube;
        Shape* xaxis;
        Shape* yaxis;
        Shape* zaxis;
        Shape* xarrow;
        Shape* yarrow;
        Shape* zarrow;
    public:
        ViewportGizmo();

        void applyRotation(Camera* sceneCam);

        Framebuffer* getFramebuffer();
        TextureRender& getTexture();
        Scene* getScene();
        Object* getObject();
    };

}

#endif /* VIEWPORTGIZMO_H */