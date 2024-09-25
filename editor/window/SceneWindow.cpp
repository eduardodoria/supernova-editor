#include "SceneWindow.h"

#include "imgui.h"
#include "external/IconsFontAwesome6.h"
#include "Platform.h"

using namespace Supernova;

Editor::SceneWindow::SceneWindow(){
    lastMousePos = Vector2(0, 0);
    draggingMouse = false;
}

void Editor::SceneWindow::sceneEventHandler(){
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

void Editor::SceneWindow::init(){
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

    gimbalXaxis->createCylinder(0.15, 2);
    gimbalYaxis->createCylinder(0.15, 2);
    gimbalZaxis->createCylinder(0.15, 2);

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
            lines->addLine(Vector3(i, gridHeight, -1000), Vector3(i, gridHeight, 1000), Vector4(0.5, 0.5, 1.0, 1.0));
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
            lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(1.0, 0.5, 0.5, 1.0));
        }else{
            if (i % 10 == 0){
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 1.0));
            }else{
                lines->addLine(Vector3(-1000, gridHeight, i), Vector3(1000, gridHeight, i), Vector4(0.5, 0.5, 0.5, 0.5));
            }
        }
    }
    lines->addLine(Vector3(0, -1000, 0), Vector3(0, 1000, 0), Vector4(0.5, 1.0, 0.5, 1.0));

    //camera->setType(CameraType::CAMERA_2D);
    camera->setPosition(10, 4, 10);
    camera->setView(0, 0, 0);

    camera->setRenderToTexture(true);
    camera->setUseFramebufferSizes(false);

    scene->setCamera(camera);
    scene->setBackgroundColor(Vector4(0.25, 0.45, 0.65, 1.0));

    Engine::setFixedTimeSceneUpdate(false);
    Engine::setScene(scene);
    Engine::addSceneLayer(sceneGimbal);
}

void Editor::SceneWindow::render(){
    if (Editor::Platform::width != 0 && Editor::Platform::height != 0){
        camera->setFramebufferSize(Editor::Platform::width, Editor::Platform::height);

        Vector3 camereworldview = camera->getWorldView();
        Vector3 view = (camera->getWorldPosition() - camera->getWorldView()).normalize();
        Vector3 right = camera->getWorldUp().crossProduct(view).normalize();
        Vector3 up = view.crossProduct(right);
        gimbal->setRotation(Quaternion(right, up, view).inverse());

        renderTexture = camera->getFramebuffer()->getRender().getColorTexture().getGLHandler();
        renderTextureGimbal = camGimbal->getFramebuffer()->getRender().getColorTexture().getGLHandler();
    }
}

void Editor::SceneWindow::show(){
    ImGui::Begin("Scene");
    {
        ImGui::BeginChild("SceneRender");
        {
            sceneEventHandler();

            width = ImGui::GetContentRegionAvail().x;
            height = ImGui::GetContentRegionAvail().y;

            if (Platform::width != width || Platform::height != height){
                Platform::width = width;
                Platform::height = height;
                Engine::systemViewChanged();
            }

            ImGui::Image((void*)(intptr_t)renderTexture, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

            // Create a new child window floating at top right
            ImVec2 childSize(100, 100); // Determined size for the new child window
            ImVec2 childPos(ImGui::GetWindowWidth() - childSize.x - 2, 2); // Position at top right with 2px padding

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

int Editor::SceneWindow::getWidth() const{
    return width;
}

int Editor::SceneWindow::getHeight() const{
    return height;
}