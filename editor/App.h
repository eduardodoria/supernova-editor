#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "window/Properties.h"
#include "window/Objects.h"
#include "window/Console.h"
#include "window/SceneWindow.h"

namespace Supernova::Editor{

    class App{
    private:
        ImGuiID dockspace_id;
        Objects objectsWindow;
        Properties propertiesWindow;
        Console consoleWindow;
        SceneWindow sceneWindow;

        static bool isInitialized;

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