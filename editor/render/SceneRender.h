#pragma once

#include "Supernova.h"

namespace Supernova::Editor{

    struct SceneProject;

    class SceneRender{
    protected:
        Scene* scene;
        Camera* camera;
        Framebuffer framebuffer;

        bool useGlobalTransform;

    public:
        SceneRender(Scene* scene);
        virtual ~SceneRender();

        virtual void activate();

        void updateRenderSystem();
        void updateSize(int width, int height);

        virtual void update(std::vector<Entity> selEntities) = 0;
        virtual void mouseHoverEvent(float x, float y) = 0;
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities) = 0;
        virtual void mouseReleaseEvent(float x, float y) = 0;
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities) = 0;

        virtual bool isAnyGizmoSideSelected() const = 0;

        TextureRender& getTexture();
        Camera* getCamera();

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
        void changeUseGlobalTransform();
    };

}