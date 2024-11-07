#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "Project.h"

#include "window/Properties.h"
#include "window/Structure.h"
#include "window/Console.h"
#include "window/SceneWindow.h"

#include "render/SceneRender.h"

namespace Supernova::Editor{

    class App{
    private:
        Project project;

        ImGuiID dockspace_id;
        ImGuiID dock_id_middle_top;

        Structure* structureWindow;
        Properties* propertiesWindow;
        Console* consoleWindow;
        SceneWindow* sceneWindow;

        static bool isInitialized;
        static bool sceneChanged;

        void showMenu();
        void showStyleEditor();
        void buildDockspace();

    public:

        App();

        void show();

        void engineInit(int argc, char** argv);
        void engineViewLoaded();
        void engineRender();
        void engineViewDestroyed();
        void engineShutdown();

        void addNewSceneToDock(uint32_t sceneId);
        void notifySceneChange();

        void kewtStyleTheme();
    };

}

#endif /* EDITORAPP_H */