#pragma once

#include "Supernova.h"

#include "ToolsLayer.h"
#include "UILayer.h"
#include "RenderUtil.h"
#include "command/Command.h"

namespace Supernova::Editor{

    struct SceneProject;

    class SceneRender{
    private:
        Plane cursorPlane;
        Vector3 rotationAxis;
        Vector3 cursorStartOffset;
        Quaternion rotationStartOffset;
        Vector3 scaleStartOffset;
        std::map<Entity, Matrix4> objectMatrixOffset;

        Ray mouseRay;
        bool mouseClicked;
        bool useGlobalTransform;

        float gizmoScale;
        float selectionOffset;

        Command* lastCommand;

        CursorSelected cursorSelected;

        AABB getFamilyAABB(Entity entity, float scale);

    protected:
        Scene* scene;
        Camera* camera;
        Framebuffer framebuffer;

        Lines* selLines;

        ToolsLayer toolslayer;
        UILayer uilayer;

        float zoom;       // current zoom level (units per pixel) for 2D

    public:
        SceneRender(Scene* scene, bool use2DGizmos, bool enableViewGizmo, float gizmoScale, float selectionOffset);
        virtual ~SceneRender();

        virtual void activate();
        virtual void updateSize(int width, int height);
        virtual void updateSelLines(AABB aabb) = 0;

        void updateRenderSystem();

        virtual void update(std::vector<Entity> selEntities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection);

        virtual bool isAnyGizmoSideSelected() const;

        TextureRender& getTexture();
        Camera* getCamera();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
        void changeUseGlobalTransform();

        void enableCursorPointer();
        void enableCursorHand();
        CursorSelected getCursorSelected();
    };

}