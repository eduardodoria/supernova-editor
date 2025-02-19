#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "Project.h"
#include "object/Camera.h"
#include <unordered_map>

namespace Supernova::Editor {

    class SceneWindow {
    private:
        Project* project;
        bool windowFocused;

        bool mouseLeftDown = false;
        Vector2 mouseLeftStartPos;
        Vector2 mouseLeftDragPos;
        bool mouseLeftDraggedInside;

        std::map<uint32_t, bool> draggingMouse;
        std::map<uint32_t, float> walkSpeed;

        std::map<uint32_t, int> width;
        std::map<uint32_t, int> height;

        std::vector<uint32_t> closeSceneQueue;

        void handleCloseScene(uint32_t sceneId);
        void sceneEventHandler(Project* project, uint32_t sceneId);
        std::string getWindowTitle(const SceneProject& sceneProject) const;
        
    public:
        SceneWindow(Project* project);

        void show();
        bool isFocused() const;

        int getWidth(uint32_t sceneId) const;
        int getHeight(uint32_t sceneId) const;
    };
}

#endif /* SCENEWINDOW_H */