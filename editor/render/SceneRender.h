#ifndef SCENERENDER_H
#define SCENERENDER_H

#include "Supernova.h"
#include "Gimbal.h"

namespace Supernova::Editor{

    class SceneRender{
    private:
        Camera* camera;
        Scene* scene;

        Gimbal gimbal;
    public:
        SceneRender(Scene* scene);

        void update(int width, int height);

        TextureRender& getTexture();

        Camera* getCamera();
        Gimbal* getGimbal();
    };

}

#endif /* SCENERENDER_H */