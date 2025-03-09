#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "ViewportGizmo.h"
#include "ToolsLayer.h"
#include "UILayer.h"
#include "command/Command.h"

namespace Supernova::Editor{

    class SceneRender3D: public SceneRender{
    private:

        Lines* lines;
        Light* sun;
        SkyBox* sky;
        Lines* selAABBLines;

        ToolsLayer toolslayer;
        UILayer uilayer;
        ViewportGizmo viewgizmo;

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
        SceneRender3D(Scene* scene);
        virtual ~SceneRender3D();

        virtual void activate();

        virtual void update(std::vector<Entity> selEntities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities);

        virtual bool isAnyGizmoSideSelected() const;

        ViewportGizmo* getViewportGizmo();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();
    };

}