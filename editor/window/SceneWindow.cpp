#include "SceneWindow.h"

#include "external/IconsFontAwesome6.h"
#include "Backend.h"
#include "Util.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"

#include "math/Vector2.h"

using namespace Supernova;

Editor::SceneWindow::SceneWindow(Project* project){
    this->project = project;
    this->mouseLeftDraggedInside = false;
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

    if (isMouseInWindow){
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            project->getScene(sceneId)->sceneRender->mouseClickEvent(x, y, project->getSelectedEntities(sceneId));
        }

        if (!(ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))){
            project->getScene(sceneId)->sceneRender->mouseHoverEvent(x, y);
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            if (!mouseLeftDown){
                mouseLeftStartPos = Vector2(x, y);
                mouseLeftDown = true;
            }
            mouseLeftDragPos = Vector2(x, y);
            if (mouseLeftStartPos.distance(mouseLeftDragPos) > 5){
                mouseLeftDraggedInside = true;
                project->getScene(sceneId)->sceneRender->mouseDragEvent(x, y, mouseLeftStartPos.x, mouseLeftStartPos.y, sceneId, project->getSelectedEntities(sceneId));
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
            if (!mouseLeftDraggedInside && mouseLeftDown){
                project->selectObjectByRay(sceneId, x, y, io.KeyShift);
            }
            mouseLeftDown = false;
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        if (mouseLeftDraggedInside && !project->getScene(sceneId)->sceneRender->isGizmoSideSelected()){
            Vector2 clickStartPos = Vector2((2 * mouseLeftStartPos.x / width[sceneId]) - 1, -((2 * mouseLeftStartPos.y / height[sceneId]) - 1));
            Vector2 clickEndPos = Vector2((2 * mouseLeftDragPos.x / width[sceneId]) - 1, -((2 * mouseLeftDragPos.y / height[sceneId]) - 1));
            project->selectObjectsByRect(sceneId, clickStartPos, clickEndPos);
        }

        project->getScene(sceneId)->sceneRender->mouseReleaseEvent(x, y);

        mouseLeftDraggedInside = false;
    }

    if (isMouseInWindow && (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
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

    bool walkingMode = false;

    // Check for mouse clicks
    if (draggingMouse[sceneId] && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)){

            walkingMode = true;

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
                camera->slide(-0.01 * mouseDelta.x);
                camera->slideUp(0.01 * mouseDelta.y);
            }else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)){
                camera->zoom(-0.1 * mouseDelta.y);
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

    if (project->getSelectedSceneId() == sceneId){
        if (!walkingMode){
            if (ImGui::IsKeyPressed(ImGuiKey_W)) {
                project->getScene(sceneId)->sceneRender->getToolsLayer()->enableTranslateGizmo();
            }

            if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                project->getScene(sceneId)->sceneRender->getToolsLayer()->enableRotateGizmo();
            }

            if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                project->getScene(sceneId)->sceneRender->getToolsLayer()->enableScaleGizmo();
            }

            if (ImGui::IsKeyPressed(ImGuiKey_T)){
                project->getScene(sceneId)->sceneRender->changeUseGlobalTransform();
            }
        }
    }


}

void Editor::SceneWindow::show(){
    for (auto& sceneProject : project->getScenes()) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin((sceneProject.name + "###Scene" + std::to_string(sceneProject.id)).c_str());
        {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)){
                project->setSelectedSceneId(sceneProject.id);
            }

            if (ImGui::Button(ICON_FA_PLAY " Play")) {
                sceneProject.name = "Testing";
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
            /*
            CursorSelected cursorSelected = sceneProject.sceneRender->getUILayer()->getCursorSelected();

            ImGui::BeginDisabled(cursorSelected == CursorSelected::POINTER);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_POINTER)) {
                sceneProject.sceneRender->getUILayer()->enableCursorPointer();
            }
            ImGui::EndDisabled();

            ImGui::BeginDisabled(cursorSelected == CursorSelected::HAND);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_HAND)) {
                sceneProject.sceneRender->getUILayer()->enableCursorHand();
            }
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);
            */

            GizmoSelected gizmoSelected = sceneProject.sceneRender->getToolsLayer()->getGizmoSelected();

            ImGui::BeginDisabled(gizmoSelected == GizmoSelected::TRANSLATE);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT)) {
                sceneProject.sceneRender->getToolsLayer()->enableTranslateGizmo();
            }
            ImGui::SetItemTooltip("Translate (W)");
            ImGui::EndDisabled();

            ImGui::BeginDisabled(gizmoSelected == GizmoSelected::ROTATE);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE)) {
                sceneProject.sceneRender->getToolsLayer()->enableRotateGizmo();
            }
            ImGui::SetItemTooltip("Rotate (E)");
            ImGui::EndDisabled();

            ImGui::BeginDisabled(gizmoSelected == GizmoSelected::SCALE);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER)) {
                sceneProject.sceneRender->getToolsLayer()->enableScaleGizmo();
            }
            ImGui::SetItemTooltip("Scale (R)");
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            bool useGlobalTransform = sceneProject.sceneRender->isUseGlobalTransform();

            ImGui::BeginDisabled(useGlobalTransform);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_GLOBE)) {
                sceneProject.sceneRender->setUseGlobalTransform(true);
            }
            ImGui::SetItemTooltip("World transform (T)");
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!useGlobalTransform);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_LOCATION_DOT)) {
                sceneProject.sceneRender->setUseGlobalTransform(false);
            }
            ImGui::SetItemTooltip("Local transform (T)");
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            if (ImGui::Button(ICON_FA_GEAR)){
                ImGui::OpenPopup("scenesettings");
            }
            if (ImGui::BeginPopup("scenesettings")){
                ImGui::Text("Scene settings");
                ImGui::Separator();

                static float test;
                ImGui::DragFloat("Shadow distance", &test);

                ImGui::EndPopup();
            }

            ImGui::BeginChild(("Canvas" + std::to_string(sceneProject.id)).c_str());
            {
                sceneEventHandler(project, sceneProject.id);

                int widthNew = ImGui::GetContentRegionAvail().x;
                int heightNew = ImGui::GetContentRegionAvail().y;

                if (widthNew != width[sceneProject.id] || heightNew != height[sceneProject.id]){
                    width[sceneProject.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneProject.id] = ImGui::GetContentRegionAvail().y;

                    sceneProject.needUpdateRender = true;
                }

                ImGui::Image((ImTextureID)(intptr_t)sceneProject.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
                if (ImGui::BeginDragDropTarget()){
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImGuiIO& io = ImGui::GetIO();
                    ImVec2 mousePos = io.MousePos;
                    float x = mousePos.x - windowPos.x;
                    float y = mousePos.y - windowPos.y;
                    Entity selEntity = project->findObjectByRay(sceneProject.id, x, y);

                    static MeshComponent* usedMesh = nullptr;
                    static Texture originalTex;

                    if (selEntity != NULL_ENTITY){
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
                            MeshComponent *mesh = sceneProject.scene->findComponent<MeshComponent>(selEntity);
                            if (mesh && receivedStrings.size() > 0){
                                if (!usedMesh){
                                    usedMesh = mesh;
                                    originalTex = mesh->submeshes[0].material.baseColorTexture;
                                }
                                Texture actualTex = mesh->submeshes[0].material.baseColorTexture;
                                Texture newTex(receivedStrings[0]);
                                if (actualTex != newTex){
                                    mesh->submeshes[0].material.baseColorTexture = Texture(newTex);
                                    mesh->needReload = true;
                                    //printf("reload\n");
                                }
                                if (payload->IsDelivery()){
                                    std::string propName = "submeshes[0].material.basecolortexture";
                                    usedMesh->submeshes[0].material.baseColorTexture = originalTex;
                                    PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(sceneProject.scene, selEntity, ComponentType::MeshComponent, propName, UpdateFlags_MeshReload, newTex);
                                    CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);

                                    usedMesh = nullptr;
                                }
                            }
                        }
                    }else{
                        if (usedMesh){
                            usedMesh->submeshes[0].material.baseColorTexture = originalTex;
                            usedMesh->needReload = true;
                            //printf("reload\n");
                            usedMesh = nullptr;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
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