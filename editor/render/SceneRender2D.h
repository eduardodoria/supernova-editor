#pragma once

#include "SceneRender.h"

#include "Supernova.h"
#include "UILayer.h"
#include "command/Command.h"

namespace Supernova::Editor{

    class SceneRender2D: public SceneRender{
    private:
        Lines* lines;
        bool isUI;

        void createLines(unsigned int width, unsigned int height);

    protected:
        void hideAllGizmos() override;

    public:
        SceneRender2D(Scene* scene, unsigned int width, unsigned int height, bool isUI);
        virtual ~SceneRender2D();

        void activate() override;
        void updateSize(int width, int height) override;
        void updateSelLines(std::vector<OBB> obbs) override;

        void update(std::vector<Entity> selEntities, std::vector<Entity> entities, Entity mainCamera) override;
        void mouseHoverEvent(float x, float y) override;
        void mouseClickEvent(float x, float y, std::vector<Entity> selEntities) override;
        void mouseReleaseEvent(float x, float y) override;
        void mouseDragEvent(float x, float y, float origX, float origY, Project* project, size_t sceneId, std::vector<Entity> selEntities, bool disableSelection) override;

        void zoomAtPosition(float width, float height, Vector2 pos, float zoomFactor);

        float getZoom() const;
    };

}