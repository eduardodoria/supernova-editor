#ifndef ANIMATIONWINDOW_H
#define ANIMATIONWINDOW_H

#include "Project.h"
#include "imgui.h"

namespace Supernova::Editor{

    class AnimationWindow{
    private:

        Project* project;

        // Playback state
        bool isPlaying;
        float currentTime;
        float playbackSpeed;

        // Timeline view state
        float pixelsPerSecond;
        float scrollX;
        float minPixelsPerSecond;
        float maxPixelsPerSecond;

        // Selection
        int selectedFrameIndex;
        bool isDraggingPlayhead;
        bool isDraggingFrame;
        int draggingFrameIndex;
        float dragStartTime;
        uint32_t dragStartTrack;

        // Snap
        bool snapToGrid;
        float snapInterval;

        // Settings
        bool autoFocusOnSelection;

        // Cached entity
        Entity selectedEntity;
        uint32_t selectedSceneId;

        // Helpers
        void drawToolbar(float width, AnimationComponent& anim, Scene* scene);
        void drawTimeRuler(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd);
        void drawTracks(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd,
                        AnimationComponent& anim, SceneProject* sceneProject);
        void drawPlayhead(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd);

        float snapTime(float time) const;
        float timeToX(float time, float timeStart, ImVec2 canvasPos) const;
        float xToTime(float x, float timeStart, ImVec2 canvasPos) const;

        std::string getActionLabel(Entity actionEntity, Scene* scene) const;
        std::string getAnimationEntityLabel(Entity entity, AnimationComponent& anim, Scene* scene) const;

    public:
        static constexpr const char* WINDOW_NAME = "Animation";

        AnimationWindow(Project* project);

        void show();

        void selectEntity(Entity entity, uint32_t sceneId);
    };

}

#endif /* ANIMATIONWINDOW_H */
