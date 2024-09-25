#include "App.h"

#include "imgui_internal.h"
#include "Platform.h"
#include "Supernova.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

bool Editor::App::isInitialized = false;

Editor::App::App(){

    lastMousePos = Vector2(0, 0);
    draggingMouse = false;

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
                // Handle undo action
            }
            if (ImGui::MenuItem("Redo")) {

                // Handle redo action
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
    ImGuiID dock_id_left, dock_id_right, dock_id_middle, dock_id_middle_top, dock_id_middle_bottom;
    float size;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split the dockspace into left and middle
    size = 12*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.0f, &dock_id_left, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_left, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set left node size
    ImGui::DockBuilderDockWindow("Objects", dock_id_left);

    // Split the middle into right and remaining middle
    size = 18*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Right, 0.0f, &dock_id_right, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_right, ImVec2(size, ImGui::GetMainViewport()->Size.y)); // Set right node size
    ImGui::DockBuilderDockWindow("Properties", dock_id_right);

    // Split the remaining middle into top and bottom
    size = 10*ImGui::GetFontSize();
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down, 0.0f, &dock_id_middle_bottom, &dock_id_middle_top);
    ImGui::DockBuilderSetNodeSize(dock_id_middle_bottom, ImVec2(ImGui::GetMainViewport()->Size.x, size)); // Set bottom node size
    ImGui::DockBuilderDockWindow("Scene", dock_id_middle_top);
    ImGui::DockBuilderDockWindow("Console", dock_id_middle_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Editor::App::sceneEventHandler(){
    // Get the current window's position and size
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 mousePos = ImGui::GetMousePos();
    ImGuiIO& io = ImGui::GetIO();
    float mouseWheel = io.MouseWheel;

    // Check if the mouse is within the window bounds
    bool isMouseInWindow = ImGui::IsWindowHovered() && (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
                            mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y);

    // Log mouse position
    if (isMouseInWindow && (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        lastMousePos = Vector2(x, y);

        draggingMouse = true;

    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle) || ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        draggingMouse = false;
    }

    // Check for mouse clicks
    if (draggingMouse && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        float difX = lastMousePos.x - x;
        float difY = lastMousePos.y - y;
        lastMousePos = Vector2(x, y);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)){

            camera->rotateView(0.1 * difX);
            camera->elevateView(0.1 * difY);

            if (ImGui::IsKeyDown(ImGuiKey_W)){
                camera->slideForward(0.05 * 10);
            }
            if (ImGui::IsKeyDown(ImGuiKey_S)){
                camera->slideForward(-0.05 * 10);
            }
            if (ImGui::IsKeyDown(ImGuiKey_A)){
                camera->slide(-0.02 * 10);
            }
            if (ImGui::IsKeyDown(ImGuiKey_D)){
                camera->slide(0.02 * 10);
            }

            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)){
            if (ImGui::IsKeyDown(ImGuiKey_ModShift)){
                camera->slide(0.01 * difX);
                camera->slideUp(-0.01 * difY);
            }else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)){
                camera->zoom(0.1 * difY);
            }else{
                camera->rotatePosition(0.1 * difX);
                camera->elevatePosition(-0.1 * difY);
            }
        }
    }

    if (isMouseInWindow && mouseWheel != 0.0f){
        camera->zoom(5 * mouseWheel);
    }
}

void Editor::App::show(){
    dockspace_id = ImGui::GetID("MyDockspace");

    showMenu();

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        buildDockspace();
    }

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

#ifdef SHOW_STYLE_WINDOW
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
#endif
    
    objectsWindow.show();
    consoleWindow.show();
    propertiesWindow.show();

    ImGui::Begin("Scene");
    {
        ImGui::BeginChild("GameRender");
        {
            sceneEventHandler();

            float width = ImGui::GetContentRegionAvail().x;
            float height = ImGui::GetContentRegionAvail().y;

            if (Platform::width != width || Platform::height != height){
                Platform::width = width;
                Platform::height = height;
                Engine::systemViewChanged();
            }

            ImGui::Image((void*)(intptr_t)renderTexture, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

            // Create a new child window floating at top right
            ImVec2 childSize(100, 100); // Determined size for the new child window
            ImVec2 childPos(ImGui::GetWindowWidth() - childSize.x - 10, 10); // Position at top right with 10px padding

            ImGui::SetCursorPos(childPos);

            ImGui::BeginChild("GimbalChild", childSize, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
            {
                ImGui::Image((void*)(intptr_t)renderTextureGimbal, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void Editor::App::engineInit(int argc, char** argv){
    Engine::systemInit(argc, argv);

    sceneGimbal = new Scene();
    camGimbal = new Camera(sceneGimbal);
    gimbal = new Object(sceneGimbal);
    gimbalcube = new Shape(sceneGimbal);
    gimbalXaxis = new Shape(sceneGimbal);
    gimbalYaxis = new Shape(sceneGimbal);
    gimbalZaxis = new Shape(sceneGimbal);
    gimbalXarrow = new Shape(sceneGimbal);
    gimbalYarrow = new Shape(sceneGimbal);
    gimbalZarrow = new Shape(sceneGimbal);

    sceneGimbal->setBackgroundColor(0.0, 0.0, 0.0, 0.0);
    sceneGimbal->setCamera(camGimbal);

    gimbalcube->createBox(0.6,0.6,0.6);
    gimbalcube->setColor(0.5, 0.5, 0.5, 1.0);

    gimbalXaxis->createCylinder(0.2, 2);
    gimbalYaxis->createCylinder(0.2, 2);
    gimbalZaxis->createCylinder(0.2, 2);

    gimbalXaxis->setColor(0.7, 0.2, 0.2, 1.0);
    gimbalYaxis->setColor(0.2, 0.7, 0.2, 1.0);
    gimbalZaxis->setColor(0.2, 0.2, 0.7, 1.0);

    gimbalXaxis->setRotation(0,0,90);
    gimbalZaxis->setRotation(90,0,0);

    gimbalXarrow->createCylinder(0.3, 0.0, 0.6);
    gimbalYarrow->createCylinder(0.3, 0.0, 0.6);
    gimbalZarrow->createCylinder(0.3, 0.0, 0.6);

    gimbalXarrow->setPosition(1.2, 0, 0);
    gimbalYarrow->setPosition(0, 1.2, 0);
    gimbalZarrow->setPosition(0, 0, 1.2);

    gimbalXarrow->setRotation(0,0,-90);
    gimbalZarrow->setRotation(90,0,0);

    gimbalXarrow->setColor(0.7, 0.2, 0.2, 1.0);
    gimbalYarrow->setColor(0.2, 0.7, 0.2, 1.0);
    gimbalZarrow->setColor(0.2, 0.2, 0.7, 1.0);

    gimbal->addChild(gimbalcube);
    gimbal->addChild(gimbalXaxis);
    gimbal->addChild(gimbalYaxis);
    gimbal->addChild(gimbalZaxis);
    gimbal->addChild(gimbalXarrow);
    gimbal->addChild(gimbalYarrow);
    gimbal->addChild(gimbalZarrow);

    camGimbal->setPosition(0, 0, 5);
    camGimbal->setView(0, 0, 0);
    camGimbal->setFramebufferSize(128, 128);
    camGimbal->setRenderToTexture(true);


    scene = new Scene();
    Lines* lines = new Lines(scene);
    camera = new Camera(scene);

    int gridHeight = 0;

    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    for (int i = -1000; i <= 1000; i++){
        if (i == 0){
            lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 1.0, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(-7, 7, 20);
    camera->setView(0, 0, 0);

    camera->setRenderToTexture(true);
    camera->setUseFramebufferSizes(false);

    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setFixedTimeSceneUpdate(false);
    Engine::setScene(scene);
    Engine::addSceneLayer(sceneGimbal);
}

void Editor::App::engineViewLoaded(){
    Engine::systemViewLoaded();
}

void Editor::App::engineRender(){
    if (Editor::Platform::width != 0 && Editor::Platform::height != 0){
        camera->setFramebufferSize(Editor::Platform::width, Editor::Platform::height);
        Vector3 camereworldview = camera->getWorldView();
        Vector3 view = (camera->getWorldPosition() - camera->getWorldView()).normalize();
        Vector3 right = camera->getWorldUp().crossProduct(view).normalize();
        Vector3 up = view.crossProduct(right);
        gimbal->setRotation(Quaternion(right, up, view));

        Engine::systemDraw();
        renderTexture = camera->getFramebuffer()->getRender().getColorTexture().getGLHandler();
        renderTextureGimbal = camGimbal->getFramebuffer()->getRender().getColorTexture().getGLHandler();
    }
}

void Editor::App::engineViewDestroyed(){
    Engine::systemViewDestroyed();
}

void Editor::App::engineShutdown(){
    Engine::systemShutdown();
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
    style.FramePadding = ImVec2(12, 6);
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