#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include "Supernova.h"

namespace Supernova::Editor{

    class SceneWindow{

    private:

        Vector2 lastMousePos;
        bool draggingMouse;

        Camera* camera;
        Scene* scene;

        Scene* sceneGimbal;
        Camera* camGimbal;
        Object* gimbal;
        Shape* gimbalcube;
        Shape* gimbalXaxis;
        Shape* gimbalYaxis;
        Shape* gimbalZaxis;
        Shape* gimbalXarrow;
        Shape* gimbalYarrow;
        Shape* gimbalZarrow;

        uint32_t renderTexture;
        uint32_t renderTextureGimbal;

        int width;
        int height;

        void sceneEventHandler();
        
    public:
        SceneWindow();

        void init();
        void render();

        void show();

        int getWidth() const;
        int getHeight() const;
    };

}

#endif /* SCENEWINDOW_H */