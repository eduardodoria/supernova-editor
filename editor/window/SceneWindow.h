#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "Project.h"
#include "object/Camera.h"

namespace Supernova::Editor{

    class SceneWindow{

    private:
        Project* project;

        bool mouseLeftDown = false;
        Vector2 mouseLeftStartPos;
        Vector2 mouseLeftDragPos;
        bool mouseLeftDraggedInside;

        std::map<uint32_t, bool> draggingMouse;
        std::map<uint32_t, float> walkSpeed;

        std::map<uint32_t, int> width;
        std::map<uint32_t, int> height;

        void sceneEventHandler(Project* project, uint32_t sceneId);
        
    public:
        SceneWindow(Project* project);

        void show();

        int getWidth(uint32_t sceneId) const;
        int getHeight(uint32_t sceneId) const;
    };

}

#endif /* SCENEWINDOW_H */