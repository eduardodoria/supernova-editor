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
        int selectedTrackIndex;
        bool isDraggingPlayhead;
        bool isDraggingTrack;
        int draggingTrackIndex;
        float dragStartTime;

        // Snap
        bool snapToGrid;
        float snapInterval;

        // Cached entity
        Entity selectedEntity;
        uint32_t selectedSceneId;

        // Helpers
        void drawToolbar(float width);
        void drawTimeRuler(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd);
        void drawTracks(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd,
                        AnimationComponent& anim, Scene* scene);
        void drawPlayhead(ImVec2 canvasPos, ImVec2 canvasSize, float timeStart, float timeEnd);

        float snapTime(float time) const;
        float timeToX(float time, float timeStart, ImVec2 canvasPos) const;
        float xToTime(float x, float timeStart, ImVec2 canvasPos) const;

        std::string getActionLabel(Entity actionEntity, Scene* scene) const;

    public:
        static constexpr const char* WINDOW_NAME = "Animation";

        AnimationWindow(Project* project);

        void show();

        void selectEntity(Entity entity, uint32_t sceneId);
    };

}

#endif /* ANIMATIONWINDOW_H */
