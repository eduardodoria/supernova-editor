#pragma once

#include "Supernova.h"

#include "ToolsLayer.h"
#include "UILayer.h"
#include "RenderUtil.h"
#include "command/Command.h"

namespace Supernova::Editor{

    struct SceneDisplaySettings {
        bool showAllJoints       = false;
        bool showAllBones        = false;
        bool hideAllBodies       = false;
        bool hideCameraView      = false;
        bool hideLightIcons      = false;
        bool hideContainerGuides = false;
        bool hideGrid            = false;
        bool hideSelectionOutline = false;
        bool showGridLines2D     = false;
        float gridSpacing2D      = 50.0f;
        float gridSpacing3D      = 1.0f;
        bool snapToGrid          = false;
    };

    class Project;

    class SceneRender{
    private:
        Plane cursorPlane;
        Vector3 rotationAxis;
        Vector3 gizmoStartPosition;
        Vector3 cursorStartOffset;
        Quaternion rotationStartOffset;
        Vector3 scaleStartOffset;
        std::map<Entity, Matrix4> objectMatrixOffset;
        std::map<Entity, Vector2> objectSizeOffset;

        Ray mouseRay;
        bool mouseClicked;
        bool useGlobalTransform;

        float gizmoScale;
        float selectionOffset;

        Command* lastCommand;

        CursorSelected cursorSelected;

        AABB getAABB(Entity entity, bool local);
        AABB getFamilyAABB(Entity entity, float offset);

        OBB getOBB(Entity entity, bool local);
        OBB getFamilyOBB(Entity entity, float offset);

    protected:
        Scene* scene;
        Camera* camera;
        Framebuffer framebuffer;

        Lines* selLines;

        ToolsLayer toolslayer;
        UILayer uilayer;

        std::vector<Scene*> childSceneLayers;

        bool multipleEntitiesSelected;
        bool isPlaying;

        SceneDisplaySettings displaySettings;

        float zoom;       // current zoom level (units per pixel) for 2D

    public:

        SceneRender(Scene* scene, bool use2DGizmos, bool enableViewGizmo, float gizmoScale, float selectionOffset);
        virtual ~SceneRender();

        virtual void hideAllGizmos();

        void setPlayMode(bool isPlaying);

        virtual void activate();
        virtual void updateSize(int width, int height);
        virtual void updateSelLines(std::vector<OBB> obbs) = 0;

        void updateRenderSystem();

        virtual void update(std::vector<Entity> selEntities, std::vector<Entity> entities, Entity mainCamera, const SceneDisplaySettings& settings = SceneDisplaySettings{});
        virtual void mouseHoverEvent(float x, float y);
        virtual void mouseClickEvent(float x, float y, std::vector<Entity> selEntities);
        virtual void mouseReleaseEvent(float x, float y);
        virtual void mouseDragEvent(float x, float y, float origX, float origY, Project* project, size_t sceneId, std::vector<Entity> selEntities, bool disableSelection);

        virtual bool isAnyGizmoSideSelected() const;

        void setChildSceneLayers(const std::vector<Scene*>& layers);

        TextureRender& getTexture();
        Camera* getCamera();
        ToolsLayer* getToolsLayer();
        UILayer* getUILayer();

        bool isUseGlobalTransform() const;
        void setUseGlobalTransform(bool useGlobalTransform);
        void changeUseGlobalTransform();

        void enableCursorPointer();
        void enableCursorHand();
        CursorSelected getCursorSelected() const;

        bool isMultipleEntitesSelected() const;

        AABB getEntitiesAABB(const std::vector<Entity>& entities);
    };

}