#include "App.h"

#include "imgui_internal.h"
#include "Platform.h"
#include "Supernova.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"

using namespace Supernova;

bool Editor::App::isInitialized = false;

Editor::App::App(){
    structureWindow = new Structure(&project);
    propertiesWindow = new Properties(&project);
    consoleWindow = new Console();
    sceneWindow = new SceneWindow(&project);
}

void Editor::App::showMenu(){
    // Remove menu bar border
    //ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Create the main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
                // Handle open action
            }
            if (ImGui::MenuItem("Save")) {
                // Handle save action
            }
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

void Editor::App::buildDockspace(){
    ImGuiID dock_id_left, dock_id_right, dock_id_middle, dock_id_middle_bottom;
    float size;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split the dockspace into left and middle
    size = 14*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.0f, &dock_id_left, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_left, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set left node size
    ImGui::DockBuilderDockWindow("Structure", dock_id_left);

    // Split the middle into right and remaining middle
    size = 19*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Right, 0.0f, &dock_id_right, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_right, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set right node size
    ImGui::DockBuilderDockWindow("Properties", dock_id_right);

    // Split the remaining middle into top and bottom
    size = 10*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down, 0.0f, &dock_id_middle_bottom, &dock_id_middle_top);
    ImGui::DockBuilderSetNodeSize(dock_id_middle_bottom, ImVec2(ImGui::GetMainViewport()->Size.x, size)); // Set bottom node size
    ImGui::DockBuilderDockWindow("Console", dock_id_middle_bottom);

    for (auto& sceneProject : project.getScenes()) {
        addNewSceneToDock(sceneProject.id);
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

void Editor::App::show(){
    ImGuiIO& io = ImGui::GetIO();
    #ifdef __APPLE__
    bool isUndo = (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_Z) && !io.KeyShift);
    bool isRedo = (io.KeySuper && ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyShift);
    #else
    bool isUndo = (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && !io.KeyShift);
    bool isRedo = (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) || (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyShift);
    #endif

    if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused()){
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            project.getSelectedScene()->sceneRender->getToolsLayer()->enableTranslateGizmo();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            project.getSelectedScene()->sceneRender->getToolsLayer()->enableRotateGizmo();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            project.getSelectedScene()->sceneRender->getToolsLayer()->enableScaleGizmo();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_T)){
            project.getSelectedScene()->sceneRender->changeUseGlobalTransform();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete)){
            project.deleteEntities(project.getSelectedSceneId(), project.getSelectedEntities(project.getSelectedSceneId()));
        }
    }

    // Update the Undo and Redo button logic:
    if (isUndo) {
        CommandHandle::get(project.getSelectedSceneId())->undo();
    }
    ImGui::SameLine();
    if (isRedo) {
        CommandHandle::get(project.getSelectedSceneId())->redo();
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

    structureWindow->show();
    consoleWindow->show();
    propertiesWindow->show();
    sceneWindow->show();
}

void Editor::App::engineInit(int argc, char** argv){
    uint32_t sceneid = project.createNewScene("New Scene");

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

            if (project.getLastActivatedSceneId() != sceneProject.id){
                sceneProject.sceneRender->activate();
                project.setLastActivatedSceneId(sceneProject.id);
                sceneChanged = true;
                #ifdef _DEBUG
                printf("DEBUG: Activated scene %u\n", project.getLastActivatedSceneId());
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