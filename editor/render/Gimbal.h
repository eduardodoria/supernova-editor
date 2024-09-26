#ifndef GIMBAL_H
#define GIMBAL_H

#include "Scene.h"
#include "object/Camera.h"
#include "object/Object.h"
#include "object/Shape.h"

namespace Supernova::Editor{

    class Gimbal{
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
        Gimbal();

        void applyRotation(Camera* sceneCam);

        TextureRender& getTexture();
        Scene* getScene();
        Object* getObject();
    };

}

#endif /* GIMBAL_H */