#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "ViewportGizmo.h"

namespace Supernova::Editor{

    class SceneRender3D: public SceneRender{
    private:

        Lines* lines;
        Light* sun;
        SkyBox* sky;

        ViewportGizmo viewgizmo;

        Vector2 linesOffset;

        void createLines();

    public:
        SceneRender3D(Scene* scene);
        virtual ~SceneRender3D();

        virtual void activate();
        virtual void updateSelLines(AABB aabb);
        virtual void updateSelLines(std::vector<OBB> obbs);

        virtual void update(std::vector<Entity> selEntities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection);

        ViewportGizmo* getViewportGizmo();
    };

}