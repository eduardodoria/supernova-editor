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

    struct AlertData{
        bool needShow = false;
        std::string title;
        std::string message;
    };

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

        bool isInitialized;

        AlertData alert;

        std::vector<std::string> droppedExternalPaths;
        bool isDroppedExternalPaths;

        uint32_t lastActivatedScene;

        enum class LastFocusedWindow {
            None,
            Scene,
            Code
        } lastFocusedWindow;

        void saveFunc();
        void saveAllFunc();

        void showMenu();
        void showAlert();
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
        void addNewCodeWindowToDock(fs::path path);

        void handleExternalDrop(const std::vector<std::string>& paths);
        void handleExternalDragEnter();
        void handleExternalDragLeave();

        void registerAlert(std::string title, std::string message);
    };

}

#endif /* EDITORAPP_H */