#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "imgui.h"
#include "Project.h"
#include "object/Camera.h"

namespace Supernova::Editor{

    class SceneWindow{

    private:

        Project* project;

        Vector2 lastMousePos;
        bool draggingMouse;

        std::map<uint32_t, int> width;
        std::map<uint32_t, int> height;

        void sceneEventHandler(Camera* camera);
        
    public:
        SceneWindow(Project* project);

        void show();

        int getWidth(uint32_t sceneId) const;
        int getHeight(uint32_t sceneId) const;
    };

}

#endif /* SCENEWINDOW_H */