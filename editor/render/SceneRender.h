#ifndef SCENERENDER_H
#define SCENERENDER_H

#include "Supernova.h"
#include "Gimbal.h"
#include "ToolsLayer.h"

namespace Supernova::Editor{

    class SceneRender{
    private:
        Camera* camera;
        Scene* scene;

        Framebuffer framebuffer;

        ToolsLayer gizmos;
        Gimbal gimbal;
    public:
        SceneRender(Scene* scene);

        void activate();
        void updateSize(int width, int height);
        void update(Entity selectedEntity);

        TextureRender& getTexture();

        Camera* getCamera();
        Gimbal* getGimbal();
        ToolsLayer* getToolsLayer();
    };

}

#endif /* SCENERENDER_H */