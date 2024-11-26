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

        Lines* lines;
        Light* sun;
        SkyBox* sky;
        Lines* selAABBLines;

        Framebuffer framebuffer;

        ToolsLayer toolslayer;
        UILayer uilayer;
        ViewportGizmo viewgizmo;

        bool useGlobalTransform;

        Vector2 linesOffset;

        Ray mouseRay;
        bool mouseClicked;
        Plane cursorPlane;
        Vector3 rotationAxis;
        Vector3 cursorStartOffset;
        Quaternion rotationStartOffset;
        Vector3 scaleStartOffset;

        std::map<Entity, Matrix4> objectMatrixOffset;

        Command* lastCommand;

        static float gizmoSize;

        AABB getFamilyAABB(Entity entity);
        void createLines();

    public:
        SceneRender(Scene* scene);

        void activate();
        void updateRenderSystem();
        void updateSize(int width, int height);
        void update(std::vector<Entity> selEntities);
        void mouseHoverEvent(float x, float y);
        void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        void mouseReleaseEvent(float x, float y);
        void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, std::vector<Entity> selEntities);

        TextureRender& getTexture();

        Camera* getCamera();
        ViewportGizmo* getViewportGizmo();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();

        bool isGizmoSideSelected() const;

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
        void changeUseGlobalTransform();
    };

}

#endif /* SCENERENDER_H */