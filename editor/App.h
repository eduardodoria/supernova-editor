#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "Supernova.h"

#include "window/Properties.h"
#include "window/Objects.h"
#include "window/Console.h"

namespace Supernova::Editor{

    class App{
    private:
        ImGuiID dockspace_id;
        Objects objectsWindow;
        Properties propertiesWindow;
        Console consoleWindow;

        Camera* camera;
        Scene* scene;
        Scene* sceneGimbal;
        Camera* camGimbal;
        Shape* gimbal;

        uint32_t renderTexture;
        uint32_t renderTextureGimbal;

        static bool isInitialized;

        Vector2 lastMousePos;
        bool draggingMouse;

        void showMenu();
        void buildDockspace();
        void sceneEventHandler();

    public:

        App();

        void show();

        void engineInit(int argc, char** argv);
        void engineViewLoaded();
        void engineRender();
        void engineViewDestroyed();
        void engineShutdown();

        void kewtStyleTheme();
    };

}

#endif /* EDITORAPP_H */