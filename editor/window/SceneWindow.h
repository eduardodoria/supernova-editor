#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "imgui.h"
#include "object/Camera.h"

namespace Supernova::Editor{

    class SceneWindow{

    private:

        Vector2 lastMousePos;
        bool draggingMouse;

        ImTextureID renderTexture;
        ImTextureID renderTextureGimbal;

        int width;
        int height;

        void sceneEventHandler(Camera* camera);
        
    public:
        SceneWindow();

        void show(Camera* camera);

        int getWidth() const;
        int getHeight() const;

        void setTexure(ImTextureID tex);
        void setGimbalTexure(ImTextureID tex);
    };

}

#endif /* SCENEWINDOW_H */