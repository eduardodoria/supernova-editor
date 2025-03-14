#pragma once

#include "Supernova.h"

#include "RenderUtil.h"

namespace Supernova::Editor{

    struct SceneProject;

    class SceneRender{
    protected:
        Scene* scene;
        Camera* camera;
        Framebuffer framebuffer;

        bool useGlobalTransform;

        CursorSelected cursorSelected;
    public:
        SceneRender(Scene* scene);
        virtual ~SceneRender();

        virtual void activate();
        virtual void updateSize(int width, int height);

        void updateRenderSystem();

        virtual void update(std::vector<Entity> selEntities) = 0;
        virtual void mouseHoverEvent(float x, float y) = 0;
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities) = 0;
        virtual void mouseReleaseEvent(float x, float y) = 0;
        virtual void mouseDragEvent(float x, float y, float origX, float origY, size_t sceneId, SceneProject* sceneProject, std::vector<Entity> selEntities, bool disableSelection) = 0;

        virtual bool isAnyGizmoSideSelected() const = 0;

        TextureRender& getTexture();
        Camera* getCamera();

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
        void changeUseGlobalTransform();

        void enableCursorPointer();
        void enableCursorHand();
        CursorSelected getCursorSelected();
    };

}