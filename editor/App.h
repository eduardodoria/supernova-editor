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

#include "window/LoadingWindow.h"

#include "window/dialog/ProjectSaveDialog.h"
#include "window/dialog/SceneSaveDialog.h"

#include "render/SceneRender.h"

#include <mutex>
#include <queue>

namespace Supernova::Editor{

    enum class AlertType {
        Info,
        Confirm,
        ThreeButton
    };

    struct AlertData{
        bool needShow = false;
        std::string title;
        std::string message;
        AlertType type = AlertType::Info;
        std::function<void()> onYes = nullptr;
        std::function<void()> onNo = nullptr;
        std::function<void()> onCancel = nullptr;
    };

    enum class SaveDialogType {
        Scene,
        Project
    };

    struct SaveDialogQueueItem {
        SaveDialogType type;
        uint32_t sceneId;  // Only used for Scene dialogs
        std::function<void()> callback = nullptr;
    };

    class App{
    private:
        Project project;

        std::mutex mainThreadTaskMutex;
        std::queue<std::function<void()>> mainThreadTasks;

        ImGuiID dockspace_id;
        ImGuiID dock_id_middle_top;

        Structure* structureWindow;
        Properties* propertiesWindow;
        OutputWindow* outputWindow;
        SceneWindow* sceneWindow;
        CodeEditor* codeEditor;
        ResourcesWindow* resourcesWindow;

        LoadingWindow* loadingWindow;

        bool isInitialized;

        AlertData alert;
        ProjectSaveDialog projectSaveDialog;
        SceneSaveDialog sceneSaveDialog;

        std::queue<SaveDialogQueueItem> saveDialogQueue;

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
        void openProjectFunc();

        void showMenu();
        void showAlert();
        void showStyleEditor();
        void buildDockspace();
        void kewtStyleTheme();
        void processNextSaveDialog();

        void processMainThreadTasks();

        void closeWindow();

    public:

        struct ThemeColors {
            static ImVec4 ButtonActivated;
            static ImVec4 FileCardBackground;
            static ImVec4 FileCardBackgroundHovered;
        };

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
        void registerThreeButtonAlert(std::string title, std::string message, std::function<void()> onYes, std::function<void()> onNo = nullptr, std::function<void()> onCancel = nullptr);
        void registerSaveSceneDialog(uint32_t sceneId, std::function<void()> callback = nullptr);
        void registerProjectSaveDialog(std::function<void()> callback = nullptr);

        // Thread-safe: schedules a task to run on the main/GL thread during the next frame.
        void enqueueMainThreadTask(std::function<void()> task);

        Project* getProject();
        const Project* getProject() const;

        CodeEditor* getCodeEditor() const;

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