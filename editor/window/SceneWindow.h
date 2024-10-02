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

        int width;
        int height;

        void sceneEventHandler(Camera* camera);
        
    public:
        SceneWindow(Project* project);

        void show();

        int getWidth() const;
        int getHeight() const;
    };

}

#endif /* SCENEWINDOW_H */