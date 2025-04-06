#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "UILayer.h"
#include "command/Command.h"

namespace Supernova::Editor{

    class SceneRender2D: public SceneRender{
    private:
        Lines* lines;

        void createLines(unsigned int width, unsigned int height);

    public:
        SceneRender2D(Scene* scene, unsigned int width, unsigned int height);
        virtual ~SceneRender2D();

        virtual void activate();
        virtual void updateSize(int width, int height);
        virtual void updateSelLines(std::vector<OBB> obbs);

        virtual void update(std::vector<Entity> selEntities);
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection);

        void zoomAtPosition(float width, float height, Vector2 pos, float zoomFactor);

        float getZoom() const;
    };

}