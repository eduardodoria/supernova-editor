#include "SceneWindow.h"

#include "external/IconsFontAwesome6.h"
#include "Backend.h"
#include "Util.h"
#include "command/CommandHandle.h"
#include "command/type/PropertyCmd.h"
#include "render/SceneRender2D.h"
#include "render/SceneRender3D.h"

#include "math/Vector2.h"

using namespace Supernova;

Editor::SceneWindow::SceneWindow(Project* project) {
    this->project = project;
    this->mouseLeftDraggedInside = false;
    this->windowFocused = false;
}

void Editor::SceneWindow::handleCloseScene(uint32_t sceneId) {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        // If we're closing the currently selected scene
        if (project->getSelectedSceneId() == sceneId) {
            // Find another scene to select
            for (const auto& otherScene : project->getScenes()) {
                if (otherScene.id != sceneId) {
                    project->setSelectedSceneId(otherScene.id);
                    ImGui::SetWindowFocus(("###Scene" + std::to_string(otherScene.id)).c_str());
                    break;
                }
            }
        }
        closeSceneQueue.push_back(sceneId);
    }
}

bool Editor::SceneWindow::isFocused() const {
    return windowFocused;
}

std::string Editor::SceneWindow::getWindowTitle(const SceneProject& sceneProject) const {
    return sceneProject.name + (sceneProject.isModified ? " *" : "") + "###Scene" + std::to_string(sceneProject.id);
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
        SceneProject* sceneProject = project->getScene(sceneId);

        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            sceneProject->sceneRender->mouseClickEvent(x, y, project->getSelectedEntities(sceneId));
        }

        if (!(ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))){
            sceneProject->sceneRender->mouseHoverEvent(x, y);
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)){
            if (!mouseLeftDown){
                mouseLeftStartPos = Vector2(x, y);
                mouseLeftDown = true;
            }
            mouseLeftDragPos = Vector2(x, y);
            if (mouseLeftStartPos.distance(mouseLeftDragPos) > 5){
                mouseLeftDraggedInside = true;
                sceneProject->sceneRender->mouseDragEvent(x, y, mouseLeftStartPos.x, mouseLeftStartPos.y, sceneId, sceneProject, project->getSelectedEntities(sceneId));
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

        if (mouseLeftDraggedInside && !project->getScene(sceneId)->sceneRender->isAnyGizmoSideSelected()){
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

    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject->sceneType == SceneType::SCENE_3D){

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
        if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused()){
            if (project->getSelectedSceneId() == sceneId){
                if (!walkingMode){
                    SceneRender3D* sceneRender3D = static_cast<SceneRender3D*>(sceneProject->sceneRender);

                    if (ImGui::IsKeyPressed(ImGuiKey_W)) {
                        sceneRender3D->getToolsLayer()->enableTranslateGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                        sceneRender3D->getToolsLayer()->enableRotateGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                        sceneRender3D->getToolsLayer()->enableScaleGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_T)){
                        project->getScene(sceneId)->sceneRender->changeUseGlobalTransform();
                    }
                }
            }
        }
    }else if (sceneProject->sceneType == SceneType::SCENE_2D){
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)){
            if (isMouseInWindow && mouseWheel != 0.0f){
                float zoomFactor = 1.0f - (0.1f * mouseWheel);

                float left = camera->getLeftClip();
                float right = camera->getRightClip();
                float bottom = camera->getBottomClip();
                float top = camera->getTopClip();

                float mouseX = mousePos.x - windowPos.x;
                float mouseY = mousePos.y - windowPos.y;

                float worldX = left + (mouseX / width[sceneProject->id]) * (right - left);
                float worldY = bottom + ((height[sceneProject->id] - mouseY) / height[sceneProject->id]) * (top - bottom);

                float currentWidth = right - left;
                float currentZoom = currentWidth / width[sceneProject->id]; // units per pixel

                float newZoom = currentZoom * zoomFactor;

                float newWidth = width[sceneProject->id] * newZoom;
                float newHeight = height[sceneProject->id] * newZoom;

                float newLeft = worldX - (mouseX / width[sceneProject->id]) * newWidth;
                float newRight = newLeft + newWidth;
                float newBottom = worldY - ((height[sceneProject->id] - mouseY) / height[sceneProject->id]) * newHeight;
                float newTop = newBottom + newHeight;

                camera->setLeftClip(newLeft);
                camera->setRightClip(newRight);
                camera->setBottomClip(newBottom);
                camera->setTopClip(newTop);

                SceneRender2D* sceneRender2D = static_cast<SceneRender2D*>(sceneProject->sceneRender);
                sceneRender2D->setZoom(newZoom);
            }
        }
    }

}

void Editor::SceneWindow::show() {
    for (uint32_t sceneId: closeSceneQueue){
        project->closeScene(sceneId);
    }
    closeSceneQueue.clear();

    windowFocused = false;

    // Iterate through all scenes in the project
    for (auto& sceneProject : project->getScenes()) {
        // Disable close button if this is the only open scene
        bool canClose = project->getScenes().size() > 1;
        bool isOpen = true;

        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin(getWindowTitle(sceneProject).c_str(), canClose ? &isOpen : nullptr)) {
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                windowFocused = true;
                project->setSelectedSceneId(sceneProject.id);
            }

            if (ImGui::Button(ICON_FA_PLAY " Play")) {
                project->build();
            }

            ImGui::BeginDisabled();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_STOP " Stop")) {
                // Handle stop button click
            }
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            if (sceneProject.sceneType == SceneType::SCENE_3D){
                SceneRender3D* sceneRender3D = (SceneRender3D*)sceneProject.sceneRender;
                GizmoSelected gizmoSelected = sceneRender3D->getToolsLayer()->getGizmoSelected();

                ImGui::BeginDisabled(gizmoSelected == GizmoSelected::TRANSLATE);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT)) {
                    sceneRender3D->getToolsLayer()->enableTranslateGizmo();
                }
                ImGui::SetItemTooltip("Translate (W)");
                ImGui::EndDisabled();

                ImGui::BeginDisabled(gizmoSelected == GizmoSelected::ROTATE);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE)) {
                    sceneRender3D->getToolsLayer()->enableRotateGizmo();
                }
                ImGui::SetItemTooltip("Rotate (E)");
                ImGui::EndDisabled();

                ImGui::BeginDisabled(gizmoSelected == GizmoSelected::SCALE);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER)) {
                    sceneRender3D->getToolsLayer()->enableScaleGizmo();
                }
                ImGui::SetItemTooltip("Scale (R)");
                ImGui::EndDisabled();
            }

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

            if (ImGui::Button(ICON_FA_GEAR)) {
                ImGui::OpenPopup("scenesettings");
            }
            if (ImGui::BeginPopup("scenesettings")) {
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

                if (widthNew != width[sceneProject.id] || heightNew != height[sceneProject.id]) {
                    width[sceneProject.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneProject.id] = ImGui::GetContentRegionAvail().y;

                    sceneProject.needUpdateRender = true;
                }

                ImGui::Image((ImTextureID)(intptr_t)sceneProject.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
                
                if (ImGui::BeginDragDropTarget()) {
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImGuiIO& io = ImGui::GetIO();
                    ImVec2 mousePos = io.MousePos;
                    float x = mousePos.x - windowPos.x;
                    float y = mousePos.y - windowPos.y;
                    Entity selEntity = project->findObjectByRay(sceneProject.id, x, y);

                    static MeshComponent* selMesh = nullptr;
                    static Texture originalTex;
                    static Entity lastSelEntity = NULL_ENTITY;

                    if (selEntity == NULL_ENTITY || lastSelEntity != selEntity) {
                        if (selMesh) {
                            if (selMesh->submeshes[0].material.baseColorTexture != originalTex) {
                                selMesh->submeshes[0].material.baseColorTexture = originalTex;
                                selMesh->needReload = true;
                            }
                            selMesh = nullptr;
                        }
                    }

                    if (selEntity != NULL_ENTITY) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                            lastSelEntity = selEntity;
                            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
                            MeshComponent* mesh = sceneProject.scene->findComponent<MeshComponent>(selEntity);
                            if (mesh && receivedStrings.size() > 0) {
                                if (!selMesh) {
                                    selMesh = mesh;
                                    originalTex = mesh->submeshes[0].material.baseColorTexture;
                                }
                                Texture newTex(receivedStrings[0]);
                                if (mesh->submeshes[0].material.baseColorTexture != newTex) {
                                    mesh->submeshes[0].material.baseColorTexture = newTex;
                                    mesh->needReload = true;
                                }
                                if (payload->IsDelivery()) {
                                    std::string propName = "submeshes[0].material.basecolortexture";
                                    selMesh->submeshes[0].material.baseColorTexture = originalTex;

                                    PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(sceneProject.scene, selEntity, ComponentType::MeshComponent, propName, UpdateFlags_MeshReload, newTex);
                                    cmd->setNoMerge();
                                    CommandHandle::get(project->getSelectedSceneId())->addCommand(cmd);

                                    ImGui::SetWindowFocus();
                                    selMesh = nullptr;
                                }
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();

        // Handle window closing
        if (!isOpen) {
            handleCloseScene(sceneProject.id);
        }
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