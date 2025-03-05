#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "imgui.h"

#include "Project.h"

#include "window/Properties.h"
#include "window/Structure.h"
#include "window/OutputWindow.h"
#include "window/SceneWindow.h"
#include "window/ResourcesWindow.h"
#include "window/CodeEditor.h"
#include "window/SceneSaveDialog.h"

#include "render/SceneRender.h"

namespace Supernova::Editor{

    enum class AlertType {
        Info,
        Confirm
    };

    struct AlertData{
        bool needShow = false;
        std::string title;
        std::string message;
        AlertType type = AlertType::Info;
        std::function<void()> onYes = nullptr;
        std::function<void()> onNo = nullptr;
    };

    class App{
    private:
        Project project;

        ImGuiID dockspace_id;
        ImGuiID dock_id_middle_top;

        Structure* structureWindow;
        Properties* propertiesWindow;
        OutputWindow* outputWindow;
        SceneWindow* sceneWindow;
        CodeEditor* codeEditor;
        ResourcesWindow* resourcesWindow;

        bool isInitialized;

        AlertData alert;
        SceneSaveDialog sceneSaveDialog;

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

        void resetLastActivatedScene();
        void updateResourcesPath();

        void registerAlert(std::string title, std::string message);
        void registerConfirmAlert(std::string title, std::string message, std::function<void()> onYes, std::function<void()> onNo = nullptr);
        void registerSaveSceneDialog(uint32_t sceneId);

        void finalizeExitAfterSave();

        // Window settings methods
        int getInitialWindowWidth() const;
        int getInitialWindowHeight() const;
        bool getInitialWindowMaximized() const;
        void saveWindowSettings(int width, int height, bool maximized);
        void initializeSettings();
        void exit();
    };

}

#endif /* EDITORAPP_H */