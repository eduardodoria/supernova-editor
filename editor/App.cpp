#include "App.h"

#include "imgui_internal.h"
#include "Platform.h"
#include "Supernova.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"

#include "Out.h"
#include "resources/fonts/fa-solid-900_ttf.h"
//#include "recources/fonts/roboto-v20-latin-regular_ttf.h"
#include "util/DefaultFont.h"

using namespace Supernova;

Editor::App::App(){
    propertiesWindow = new Properties(&project);
    outputWindow = new OutputWindow();
    sceneWindow = new SceneWindow(&project);
    structureWindow = new Structure(&project, sceneWindow);
    codeEditor = new CodeEditor();
    resourcesWindow = new ResourcesWindow(&project, codeEditor);

    isInitialized = false;

    lastFocusedWindow = LastFocusedWindow::None;

    Out::setOutputWindow(outputWindow);

    isDroppedExternalPaths = false;

    resetLastActivatedScene();
}

void Editor::App::saveFunc(){
    if (lastFocusedWindow == LastFocusedWindow::Scene) {
        project.saveLastSelectedScene();
    } else if (lastFocusedWindow == LastFocusedWindow::Code) {
        codeEditor->saveLastFocused();
    }
}

void Editor::App::saveAllFunc(){
    project.saveAllScenes();
    codeEditor->saveAll();
}

void Editor::App::showMenu(){
    // Remove menu bar border
    //ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Create the main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                project.createEmptyProject("MySupernovaProject");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
                project.openProject();
            }
            ImGui::BeginDisabled(!project.isTempPath());
            if (ImGui::MenuItem("Save Project")) {
                project.saveProject(true);
            }
            ImGui::EndDisabled();
            ImGui::Separator();
            bool canSave = false;
            if (lastFocusedWindow == LastFocusedWindow::Scene) {
                canSave = project.hasSelectedSceneUnsavedChanges();
            } else if (lastFocusedWindow == LastFocusedWindow::Code) {
                canSave = codeEditor->hasLastFocusedUnsavedChanges();
            }
            bool canSaveAll = project.hasScenesUnsavedChanges() || codeEditor->hasUnsavedChanges();

            ImGui::BeginDisabled(!canSave);
            if (ImGui::MenuItem("Save")) {
                saveFunc();
            }
            ImGui::EndDisabled();
            ImGui::BeginDisabled(!canSaveAll);
            if (ImGui::MenuItem("Save All")) {
                saveAllFunc();
            }
            ImGui::EndDisabled();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Handle exit action
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                CommandHandle::get(project.getSelectedSceneId())->undo();
            }
            if (ImGui::MenuItem("Redo")) {
                CommandHandle::get(project.getSelectedSceneId())->redo();
            }
            if (ImGui::MenuItem("Reset layout")) {
                buildDockspace();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Restore previous style
    //ImGui::PopStyleVar(2);
}

void Editor::App::showAlert(){
    if (alert.needShow) {
        ImGui::OpenPopup((alert.title + "##AlertModal").c_str());

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // Make sure we're using the correct flags
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                                ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoSavedSettings |
                                ImGuiWindowFlags_Modal;

        if (ImGui::BeginPopupModal((alert.title + "##AlertModal").c_str(), nullptr, flags)) {
            ImGui::Text("%s", alert.message.c_str());
            ImGui::Separator();

            if (alert.type == AlertType::Info) {
                // For info alerts, just show OK button
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 120) * 0.5f);
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    alert.needShow = false;
                    ImGui::CloseCurrentPopup();
                }
            } else if (alert.type == AlertType::Confirm) {
                // For confirmation alerts, show Yes and No buttons
                float windowWidth = ImGui::GetWindowSize().x;
                float buttonsWidth = 250; // Total width for both buttons and spacing
                ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    alert.needShow = false;
                    ImGui::CloseCurrentPopup();
                    if (alert.onYes) {
                        alert.onYes();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0))) {
                    alert.needShow = false;
                    ImGui::CloseCurrentPopup();
                    if (alert.onNo) {
                        alert.onNo();
                    }
                }
            }
            ImGui::EndPopup();
        }
    }
}

void Editor::App::buildDockspace(){
    ImGuiID dock_id_left, dock_id_left_top, dock_id_left_bottom, dock_id_right, dock_id_middle, dock_id_middle_bottom;
    float size;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split the dockspace into left and middle
    size = 14*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.0f, &dock_id_left, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_left, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set left node size
    ImGui::DockBuilderDockWindow("Structure", dock_id_left);

    size = 50*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.0f, &dock_id_left_bottom, &dock_id_left_top);
    ImGui::DockBuilderSetNodeSize(dock_id_left_bottom, ImVec2(ImGui::GetMainViewport()->Size.x, size));
    ImGui::DockBuilderDockWindow("Resources", dock_id_left_bottom);

    // Split the middle into right and remaining middle
    size = 19*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Right, 0.0f, &dock_id_right, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_right, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set right node size
    ImGui::DockBuilderDockWindow("Properties", dock_id_right);

    // Split the remaining middle into top and bottom
    size = 10*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down, 0.0f, &dock_id_middle_bottom, &dock_id_middle_top);
    ImGui::DockBuilderSetNodeSize(dock_id_middle_bottom, ImVec2(ImGui::GetMainViewport()->Size.x, size)); // Set bottom node size
    ImGui::DockBuilderDockWindow("Output", dock_id_middle_bottom);

    for (auto& sceneProject : project.getScenes()) {
        addNewSceneToDock(sceneProject.id);
    }

    for (auto& codePath : codeEditor->getOpenPaths()) {
        addNewCodeWindowToDock(codePath);
    }

    ImGui::DockBuilderFinish(dockspace_id);
}

void Editor::App::showStyleEditor(){
    ImGui::Begin("Dear ImGui Style Editor", nullptr);
    {
        // Get the current IO object to access display size
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Calculate the centered position at the top
        float windowX = (io.DisplaySize.x - windowSize.x - 200) / 2;
        float windowY = 0.0f; // Top of the screen

        // Set the window position
        ImGui::SetWindowPos(ImVec2(windowX, windowY), ImGuiCond_Once);
        ImGui::SetWindowCollapsed(true, ImGuiCond_Once);

        ImGui::ShowStyleEditor();
    }
    ImGui::End();
}

void Editor::App::setup(){
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

    io.ConfigWindowsMoveFromTitleBarOnly = true;

    #ifdef _DEBUG
    io.IniFilename = nullptr;  // Disable saving to ini file
    #endif

    io.Fonts->AddFontDefault();

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 16.0f;
    config.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, 16.0f, &config, icon_ranges);

    ImFontConfig config1;
    strcpy(config1.Name, "roboto-v20-latin-regular (16 px)");
    config1.FontDataOwnedByAtlas = false;
    config1.OversampleH = 2;
    config1.OversampleV = 2;
    config1.RasterizerMultiply = 1.5f;
    ImFont* font1 = io.Fonts->AddFontFromMemoryTTF(roboto_v20_latin_regular_ttf, roboto_v20_latin_regular_ttf_len, 16.0f, &config1);

    ImFontConfig config2;
    config2.MergeMode = true;
    config2.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
    config2.FontDataOwnedByAtlas = false;
    static const ImWchar icon_ranges2[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromMemoryTTF(fa_solid_900_ttf, fa_solid_900_ttf_len, 16.0f, &config2, icon_ranges2);

    io.FontDefault = font1;

    io.ConfigDragClickToInputText = true;

    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    //ImGui::StyleColorsDark();
    kewtStyleTheme();
}

void Editor::App::show(){
    if (sceneWindow->isFocused()) {
        lastFocusedWindow = LastFocusedWindow::Scene;
    } else if (codeEditor->isFocused()) {
        lastFocusedWindow = LastFocusedWindow::Code;
    }

    ImGuiIO& io = ImGui::GetIO();
    bool isUndo = (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && !io.KeyShift);
    bool isRedo = (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) || (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyShift);

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (ImGui::GetIO().KeyShift) {
            // CTRL+SHIFT+S saves all files
            saveAllFunc();
        } else {
            saveFunc();
        }
    }

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        // CTRL+O opens a project
        project.openProject();
    }

    if (isDroppedExternalPaths) {
        isDroppedExternalPaths = false;
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern)) {
            ImGui::SetDragDropPayload("external_files", &droppedExternalPaths, sizeof(std::vector<std::string>));
            ImGui::EndDragDropSource();
        }
    }

    if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused()){
        // space to keys events
    }

    if (!resourcesWindow->isFocused() && !codeEditor->isFocused()){
        uint32_t sceneId = project.getSelectedSceneId();

        // Update the Undo and Redo button logic:
        if (isUndo) {
            CommandHandle::get(sceneId)->undo();
        }
        ImGui::SameLine();
        if (isRedo) {
            CommandHandle::get(sceneId)->redo();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete)){
            project.deleteEntities(sceneId, project.getSelectedEntities(sceneId));
        }
    }

    dockspace_id = ImGui::GetID("MyDockspace");

    showMenu();

    isInitialized = true;

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        buildDockspace();
    }

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    #ifdef SHOW_STYLE_WINDOW
    showStyleEditor();
    #endif

    showAlert();

    structureWindow->show();
    resourcesWindow->show();
    outputWindow->show();
    propertiesWindow->show();
    codeEditor->show();
    sceneWindow->show();
}

void Editor::App::engineInit(int argc, char** argv){
    project.createTempProject("MySupernovaProject");

    System::setExternalSystem(new Editor::Platform());
    Engine::systemInit(argc, argv);
}

void Editor::App::engineViewLoaded(){
    Engine::systemViewLoaded();
}

void Editor::App::engineRender(){
    for (auto& sceneProject : project.getScenes()) {
        if (sceneProject.needUpdateRender || sceneProject.id == project.getSelectedSceneId()){
            int width = sceneWindow->getWidth(sceneProject.id);
            int height = sceneWindow->getHeight(sceneProject.id);

            SceneRender* sceneRender = sceneProject.sceneRender;

            bool sceneChanged = false;

            if (lastActivatedScene != sceneProject.id){
                sceneProject.sceneRender->activate();
                lastActivatedScene = sceneProject.id;
                sceneChanged = true;
                #ifdef _DEBUG
                printf("DEBUG: Activated scene %u\n", lastActivatedScene);
                #endif
            }

            if (width != 0 && height != 0){
                if (Platform::setSizes(width, height) || sceneChanged){
                    Engine::systemViewChanged();
                    sceneRender->updateSize(width, height); //TODO: not been used
                    sceneChanged = false;
                }

                // to avoid delay when move objects with gizmo
                sceneRender->updateRenderSystem();

                //TODO: avoid calling every frame
                sceneRender->update(project.getSelectedEntities(sceneProject.id));

                Engine::systemDraw();

                sceneProject.needUpdateRender = false;
            }
        }
    }
}

void Editor::App::engineViewDestroyed(){
    Engine::systemViewDestroyed();
}

void Editor::App::engineShutdown(){
    Engine::systemShutdown();
}

void Editor::App::addNewSceneToDock(uint32_t sceneId){
    if (isInitialized){
        ImGui::DockBuilderDockWindow(("###Scene" + std::to_string(sceneId)).c_str(), dock_id_middle_top);
    }
}

void Editor::App::addNewCodeWindowToDock(fs::path path){
    if (isInitialized){
        ImGui::DockBuilderDockWindow(("###" + path.string()).c_str(), dock_id_middle_top);
    }
}

void Editor::App::kewtStyleTheme(){
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.75f, 0.73f, 0.73f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.16f, 0.15f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.16f, 0.15f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.11f, 0.10f, 0.09f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.11f, 0.10f, 0.09f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.11f, 0.10f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.16f, 0.15f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.16f, 0.15f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.33f, 0.41f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.33f, 0.41f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.39f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.28f, 0.33f, 0.41f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.34f, 0.40f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.39f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_TabSelected]            = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline]    = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_TabDimmed]              = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]      = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline]  = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.23f, 0.51f, 0.96f, 0.78f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.27f, 0.25f, 0.24f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.47f, 0.44f, 0.42f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.34f, 0.33f, 0.31f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink]               = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.23f, 0.51f, 0.96f, 0.38f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // main
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(10, 4);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(8, 8);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 16;
    style.ScrollbarSize = 12;
    style.GrabRounding = 12;

    // borders
    style.WindowBorderSize = 0;
    style.ChildBorderSize = 0;
    style.PopupBorderSize = 0;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    style.TabBarBorderSize = 1;
    style.TabBarOverlineSize = 2;

    // rounding
    style.WindowRounding = 2;
    style.ChildRounding = 2;
    style.FrameRounding = 2;
    style.PopupRounding = 2;
    style.ScrollbarRounding = 2;
    style.GrabRounding = 2;
    style.TabRounding = 2;

    // tables
    style.CellPadding = ImVec2(8, 4);
    style.TableAngledHeadersAngle = 0.61; //35 deg
    style.TableAngledHeadersTextAlign = ImVec2(0.50, 0.00);

    // widgets
    style.WindowTitleAlign = ImVec2(0.00, 0.50);
    style.WindowMenuButtonPosition = ImGuiDir::ImGuiDir_Right;
    style.ColorButtonPosition = ImGuiDir::ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.50, 0.50);
    style.SelectableTextAlign = ImVec2(0.00, 0.00);
    style.SeparatorTextBorderSize = 1;
    style.SeparatorTextAlign = ImVec2(0.00, 0.50);
    style.SeparatorTextPadding = ImVec2(16, 0);
    style.LogSliderDeadzone = 4;

    // docking
    style.DockingSeparatorSize = 6;
}

void Editor::App::handleExternalDrop(const std::vector<std::string>& paths) {
    isDroppedExternalPaths = true;
    droppedExternalPaths = paths;
}

void Editor::App::handleExternalDragEnter() {
    resourcesWindow->handleExternalDragEnter();
}

void Editor::App::handleExternalDragLeave() {
    resourcesWindow->handleExternalDragLeave();
}

void Editor::App::resetLastActivatedScene(){
    lastActivatedScene = NULL_PROJECT_SCENE;
}

void Editor::App::registerConfirmAlert(std::string title, std::string message, std::function<void()> onYes, std::function<void()> onNo) {
    alert.needShow = true;
    alert.title = title;
    alert.message = message;
    alert.type = AlertType::Confirm;
    alert.onYes = onYes;
    alert.onNo = onNo;
}

void Editor::App::registerAlert(std::string title, std::string message) {
    alert.needShow = true;
    alert.title = title;
    alert.message = message;
    alert.type = AlertType::Info;
    alert.onYes = nullptr;
    alert.onNo = nullptr;
}