#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "Project.h"

#include "window/Properties.h"
#include "window/Structure.h"
#include "window/Output.h"
#include "window/SceneWindow.h"
#include "window/ResourcesWindow.h"
#include "window/CodeEditor.h"

#include "render/SceneRender.h"

namespace Supernova::Editor{

    class App{
    private:
        Project project;

        ImGuiID dockspace_id;
        ImGuiID dock_id_middle_top;

        Structure* structureWindow;
        Properties* propertiesWindow;
        Output* outputWindow;
        SceneWindow* sceneWindow;
        CodeEditor* codeEditor;
        ResourcesWindow* resourcesWindow;

        static bool isInitialized;

        std::vector<std::string> droppedExternalPaths;
        bool isDroppedExternalPaths;

        void showMenu();
        void showStyleEditor();
        void buildDockspace();
        void kewtStyleTheme();

    public:

        App();

        void setup();

        void show();

        void engineInit(int argc, char** argv);
        void engineViewLoaded();
        void engineRender();
        void engineViewDestroyed();
        void engineShutdown();

        void addNewSceneToDock(uint32_t sceneId);

        void handleExternalDrop(const std::vector<std::string>& paths);
        void handleExternalDragEnter();
        void handleExternalDragLeave();
    };

}

#endif /* EDITORAPP_H */