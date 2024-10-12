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

        Ray mouseRay;
        bool mouseClicked;
        Plane cursorPlane;
        Vector3 objectOffset;

        static float gizmoSize;

    public:
        SceneRender(Scene* scene);

        void activate();
        void updateSize(int width, int height);
        void update(Entity selectedEntity);
        void mouseHoverEvent(float x, float y);
        void mouseClickEvent(float x, float y, Entity entity);
        void mouseReleaseEvent(float x, float y);
        void mouseDragEvent(float x, float y, Entity entity);

        TextureRender& getTexture();

        Camera* getCamera();
        ViewportGizmo* getViewportGizmo();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();

        bool isGizmoSelected() const;
    };

}

#endif /* SCENERENDER_H */