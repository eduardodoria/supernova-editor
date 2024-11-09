#ifndef SCENERENDER_H
#define SCENERENDER_H

#include "Supernova.h"
#include "ViewportGizmo.h"
#include "ToolsLayer.h"
#include "UILayer.h"
#include "command/Command.h"

namespace Supernova::Editor{

    class SceneRender{
    private:
        Camera* camera;
        Scene* scene;

        Framebuffer framebuffer;

        ToolsLayer toolslayer;
        UILayer uilayer;
        ViewportGizmo viewgizmo;

        bool useGlobalTransform;

        Ray mouseRay;
        bool mouseClicked;
        Vector2 mouseStartPosition;
        Plane cursorPlane;
        Vector3 cursorStartOffset;
        Quaternion rotationStartOffset;
        Vector3 scaleStartOffset;

        Command* lastCommand;

        static float gizmoSize;

    public:
        SceneRender(Scene* scene);

        void activate();
        void updateRenderSystem();
        void updateSize(int width, int height);
        void update(Entity selectedEntity);
        void mouseHoverEvent(float x, float y);
        void mouseClickEvent(float x, float y, Entity selEntity);
        void mouseReleaseEvent(float x, float y);
        void mouseDragEvent(float x, float y, Entity selEntity);

        TextureRender& getTexture();

        Camera* getCamera();
        ViewportGizmo* getViewportGizmo();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();

        bool isGizmoSideSelected() const;

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
    };

}

#endif /* SCENERENDER_H */