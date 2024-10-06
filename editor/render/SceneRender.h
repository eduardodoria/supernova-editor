#ifndef SCENERENDER_H
#define SCENERENDER_H

#include "Supernova.h"
#include "Gimbal.h"
#include "Gizmos.h"

namespace Supernova::Editor{

    class SceneRender{
    private:
        Camera* camera;
        Scene* scene;
        Shape* cube;

        Gizmos gizmos;
        Gimbal gimbal;
    public:
        SceneRender(Scene* scene);

        void activate();
        void updateSize(int width, int height);
        void update();

        TextureRender& getTexture();

        Camera* getCamera();
        Gimbal* getGimbal();
        Gizmos* getGizmos();
    };

}

#endif /* SCENERENDER_H */