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

        ImTextureID renderTexture;
        ImTextureID renderTextureGimbal;

        void sceneEventHandler(Camera* camera);
        
    public:
        SceneWindow(Project* project);

        void show(Camera* camera);

        int getWidth() const;
        int getHeight() const;

        void setTexure(ImTextureID tex);
        void setGimbalTexure(ImTextureID tex);
    };

}

#endif /* SCENEWINDOW_H */