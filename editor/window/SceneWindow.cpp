#include "SceneWindow.h"

#include "external/IconsFontAwesome6.h"
#include "backend/BackendGLFW.h"

#include "math/Vector2.h"

using namespace Supernova;

Editor::SceneWindow::SceneWindow(Project* project){
    this->project = project;
}

void Editor::SceneWindow::sceneEventHandler(Project* project, uint32_t sceneId){
    // Get the current window's position and size
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuiIO& io = ImGui::GetIO();
    float mouseWheel = io.MouseWheel;
    ImVec2 mousePos = io.MousePos;
    ImVec2 mouseDelta = io.MouseDelta;

    // Check if the mouse is within the window bounds
    bool isMouseInWindow = ImGui::IsWindowHovered() && (mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + windowSize.x &&
                            mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y);

    if (isMouseInWindow && (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        draggingMouse[sceneId] = true;

        ImGui::SetWindowFocus();
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle) || ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        draggingMouse[sceneId] = false;
        Backend::enableMouseCursor();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }

    Camera* camera = project->getScene(sceneId)->sceneRender->getCamera();

    // Check for mouse clicks
    if (draggingMouse[sceneId] && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)){

            camera->rotateView(-0.1 * mouseDelta.x);
            camera->elevateView(-0.1 * mouseDelta.y);

            float minSpeed = 0.5;
            float maxSpeed = 1000;
            float speedOffset = 10.0;

            walkSpeed[sceneId] += mouseWheel;
            if (walkSpeed[sceneId] <= -speedOffset){
                walkSpeed[sceneId] = -speedOffset + minSpeed;
            }
            if (walkSpeed[sceneId] > maxSpeed){
                walkSpeed[sceneId] = maxSpeed;
            }

            float finalSpeed = 0.02 * (speedOffset + walkSpeed[sceneId]);

            if (ImGui::IsKeyDown(ImGuiKey_W)){
                camera->slideForward(finalSpeed);
            }
            if (ImGui::IsKeyDown(ImGuiKey_S)){
                camera->slideForward(-finalSpeed);
            }
            if (ImGui::IsKeyDown(ImGuiKey_A)){
                camera->slide(-finalSpeed);
            }
            if (ImGui::IsKeyDown(ImGuiKey_D)){
                camera->slide(finalSpeed);
            }
            if (ImGui::IsKeyDown(ImGuiKey_E)){
                camera->slideUp(finalSpeed);
            }
            if (ImGui::IsKeyDown(ImGuiKey_Q)){
                camera->slideUp(-finalSpeed);
            }

            //ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            Backend::disableMouseCursor();
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)){
            if (ImGui::IsKeyDown(ImGuiKey_ModShift)){
                camera->slide(0.01 * mouseDelta.x);
                camera->slideUp(-0.01 * mouseDelta.y);
            }else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)){
                camera->zoom(0.1 * mouseDelta.y);
            }else{
                camera->rotatePosition(-0.1 * mouseDelta.x);
                camera->elevatePosition(0.1 * mouseDelta.y);
            }
        }
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)){
        if (isMouseInWindow && mouseWheel != 0.0f){
            camera->zoom(2.0 * mouseWheel);
        }
    }
}

void Editor::SceneWindow::show(){
    for (auto& sceneData : project->getScenes()) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin((sceneData.name + "###Scene" + std::to_string(sceneData.id)).c_str());
        {

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)){
                project->setSelectedSceneId(sceneData.id);
            }

            if (ImGui::Button(ICON_FA_PLAY " Play")) {
                sceneData.name = "Testing";
                // Handle play button click
            }

            ImGui::BeginDisabled();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_STOP " Stop")) {
                // Handle play button click
            }
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_POINTER)) {
                // Handle play button click
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_HAND)) {
                // Handle play button click
            }

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT)) {
                // Handle play button click
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE)) {
                // Handle play button click
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER)) {
                // Handle play button click
            }

            ImGui::BeginChild(("Canvas" + std::to_string(sceneData.id)).c_str());
            {
                sceneEventHandler(project, sceneData.id);

                int widthNew = ImGui::GetContentRegionAvail().x;
                int heightNew = ImGui::GetContentRegionAvail().y;

                if (widthNew != width[sceneData.id] || heightNew != height[sceneData.id]){
                    width[sceneData.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneData.id] = ImGui::GetContentRegionAvail().y;

                    sceneData.needUpdateRender = true;
                }

                ImGui::Image((void*)(intptr_t)sceneData.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

                ImGui::SetCursorPos(ImVec2(0, 0));

                // Create a new child window covering the entire Canvas
                ImGui::BeginChild("MiddleChild", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
                {
                    ImGui::Image((void*)(intptr_t)sceneData.sceneRender->getGizmos()->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
                }
                ImGui::EndChild();

                // Create a new child window floating at top right
                ImVec2 childSize(100, 100); // Determined size for the new child window
                ImVec2 childPos(ImGui::GetWindowWidth() - childSize.x - 2, 2); // Position at top right with 2px padding

                ImGui::SetCursorPos(childPos);

                ImGui::BeginChild("GimbalChild", childSize, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
                {
                    ImGui::Image((void*)(intptr_t)sceneData.sceneRender->getGimbal()->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

int Editor::SceneWindow::getWidth(uint32_t sceneId) const{
    if (width.count(sceneId)){
        return width.at(sceneId);
    }

    return 0;
}

int Editor::SceneWindow::getHeight(uint32_t sceneId) const{
    if (height.count(sceneId)){
        return height.at(sceneId);
    }

    return 0;
}