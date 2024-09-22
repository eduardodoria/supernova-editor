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
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

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
    ImGui::PopStyleVar(2);
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
                camera->moveForward(0.05 * 10);
            }
            if (ImGui::IsKeyDown(ImGuiKey_S)){
                camera->moveForward(-0.05 * 10);
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
            camera->rotatePosition(0.1 * difX);
            camera->elevatePosition(-0.1 * difY);

            if (mouseWheel != 0.0f){
                camera->zoom(5 * mouseWheel);
            }
        }
    }
}

void Editor::App::show(){
    dockspace_id = ImGui::GetID("MyDockspace");

    showMenu();

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        buildDockspace();
    }

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

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
        }
        ImGui::EndChild();
    }
    ImGui::End();


    ImGui::Begin("Dear ImGui Style Editor");
    // Get the current IO object to access display size
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Calculate the centered position at the top
    float windowX = (io.DisplaySize.x - windowSize.x) / 2;
    float windowY = 0.0f; // Top of the screen

    // Set the window position
    ImGui::SetWindowPos(ImVec2(windowX, windowY), ImGuiCond_Once);
    ImGui::SetWindowCollapsed(true, ImGuiCond_Once);

    ImGui::ShowStyleEditor();
    ImGui::End();
    
    objectsWindow.show();
    consoleWindow.show();
    propertiesWindow.show();
}

void Editor::App::engineInit(int argc, char** argv){
    Engine::systemInit(argc, argv);

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
    camera->setPosition(0, 7, 20);
    camera->setView(0, 2, 0);

    camera->setRenderToTexture(true);
    camera->setUseFramebufferSizes(false);

    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setFixedTimeSceneUpdate(false);
    Engine::setScene(scene);
}

void Editor::App::engineViewLoaded(){
    Engine::systemViewLoaded();
}

void Editor::App::engineRender(){
    if (Editor::Platform::width != 0 && Editor::Platform::height != 0){
        camera->setFramebufferSize(Editor::Platform::width, Editor::Platform::height);
        Engine::systemDraw();
        renderTexture = camera->getFramebuffer()->getRender().getColorTexture().getGLHandler();
    }
}

void Editor::App::engineViewDestroyed(){
    Engine::systemViewDestroyed();
}

void Editor::App::engineShutdown(){
    Engine::systemShutdown();
}
