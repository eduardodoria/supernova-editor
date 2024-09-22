#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "Supernova.h"

#include "layout/Properties.h"
#include "layout/Objects.h"
#include "layout/Console.h"

namespace Supernova::Editor{

    class App{
    private:
        ImGuiID dockspace_id;
        Objects objectsWindow;
        Properties propertiesWindow;
        Console consoleWindow;

        Camera* camera;
        Scene* scene;

        uint32_t renderTexture;

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
    };

}

#endif /* EDITORAPP_H */