#ifndef SCENERENDER_H
#define SCENERENDER_H

#include "Supernova.h"
#include "ViewportGizmo.h"
#include "ToolsLayer.h"
#include "UILayer.h"

namespace Supernova::Editor{

    class SceneRender{
    private:
        Camera* camera;
        Scene* scene;

        Framebuffer framebuffer;

        ToolsLayer toolslayer;
        UILayer uilayer;
        ViewportGizmo viewgizmo;
    public:
        SceneRender(Scene* scene);

        void activate();
        void updateSize(int width, int height);
        void update(Entity selectedEntity);

        TextureRender& getTexture();

        Camera* getCamera();
        ViewportGizmo* getViewportGizmo();
        ToolsLayer* getToolsLayer();
    };

}

#endif /* SCENERENDER_H */