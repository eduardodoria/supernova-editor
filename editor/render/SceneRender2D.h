#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "ViewportGizmo.h"
#include "ToolsLayer.h"
#include "UILayer.h"
#include "command/Command.h"

namespace Supernova::Editor{

    class SceneRender2D: public SceneRender{
    private:
        Lines* lines;

        float zoom;       // Current zoom level (units per pixel)
    public:
        SceneRender2D(Scene* scene);
        virtual ~SceneRender2D();

        virtual void activate();
        virtual void updateSize(int width, int height);

        virtual void update(std::vector<Entity> selEntities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities);

        virtual bool isAnyGizmoSideSelected() const;

        void setZoom(float newZoom);
        float getZoom() const;
    };

}