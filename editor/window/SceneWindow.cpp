#include "SceneWindow.h"

#include "external/IconsFontAwesome6.h"
#include "Platform.h"

#include "math/Vector2.h"

using namespace Supernova;

Editor::SceneWindow::SceneWindow(Project* project){
    this->project = project;

    lastMousePos = Vector2(0, 0);
    draggingMouse = false;
}

void Editor::SceneWindow::sceneEventHandler(Camera* camera){
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

void Editor::SceneWindow::show(){
    for (auto& sceneData : project->getScenes()) {
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
                if (project->getSelectedSceneId() == sceneData.id){
                    sceneEventHandler(sceneData.sceneRender->getCamera());
                }

                int widthNew = ImGui::GetContentRegionAvail().x;
                int heightNew = ImGui::GetContentRegionAvail().y;

                if (widthNew != width[sceneData.id] || heightNew != height[sceneData.id]){
                    width[sceneData.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneData.id] = ImGui::GetContentRegionAvail().y;

                    sceneData.needUpdateRender = true;
                }

                ImGui::Image((void*)(intptr_t)sceneData.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

                // Create a new child window floating at top right
                ImVec2 childSize(100, 100); // Determined size for the new child window
                ImVec2 childPos(ImGui::GetWindowWidth() - childSize.x - 2, 2); // Position at top right with 2px padding

                ImGui::SetCursorPos(childPos);

                ImGui::BeginChild("GimbalChild", childSize, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
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