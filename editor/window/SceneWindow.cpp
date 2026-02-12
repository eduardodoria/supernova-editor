#include "SceneWindow.h"

#include "Input.h"
#include "external/IconsFontAwesome6.h"
#include "Backend.h"
#include "util/Util.h"
#include "command/CommandHandle.h"
#include "command/type/MultiPropertyCmd.h"
#include "command/type/CreateEntityCmd.h"
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
        project->checkUnsavedAndExecute(sceneId, [this, sceneId]() {
            closeSceneInternal(sceneId);
        });
    }
}

void Editor::SceneWindow::closeSceneInternal(uint32_t sceneId) {
    SceneProject* sceneProject = project->getScene(sceneId);
    if (sceneProject) {
        // If we're closing the currently selected scene
        if (project->getSelectedSceneId() == sceneId) {
            // Find another scene to select
            for (const auto& otherScene : project->getScenes()) {
                if (otherScene.id != sceneId && otherScene.opened) {
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
    std::string icon;
    if (sceneProject.sceneType == SceneType::SCENE_3D){
        icon = ICON_FA_CUBES + std::string("  ");
    }else if (sceneProject.sceneType == SceneType::SCENE_2D){
        icon = ICON_FA_CUBES_STACKED + std::string("  ");
    }else if (sceneProject.sceneType == SceneType::SCENE_UI){
        icon = ICON_FA_WINDOW_RESTORE + std::string("  ");
    }
    return icon + sceneProject.name + ((project->hasSceneUnsavedChanges(sceneProject.id)) ? " *" : "") + "###Scene" + std::to_string(sceneProject.id);
}

void Editor::SceneWindow::handleResourceFileDragDrop(SceneProject* sceneProject) {
    static bool draggingResourceFile = false;
    static Image* tempImage = nullptr;
    static MeshComponent* selMesh = nullptr;
    static UIComponent* selUI = nullptr;
    static TextComponent* selText = nullptr;
    static Texture originalTex;
    static std::string originalFont;
    static Entity lastSelEntity = NULL_ENTITY;

    if (ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* peekPayload = ImGui::GetDragDropPayload();
        if (peekPayload && peekPayload->IsDataType("resource_files")) {
            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(peekPayload);
            if (receivedStrings.size() > 0) {
                const std::string droppedRelativePath = std::filesystem::relative(receivedStrings[0], project->getProjectPath()).generic_string();
                bool isFont = Util::isFontFile(droppedRelativePath);
                bool isImage = Util::isImageFile(droppedRelativePath);

                draggingResourceFile = true;
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 mousePos = io.MousePos;
                float x = mousePos.x - windowPos.x;
                float y = mousePos.y - windowPos.y;
                Entity selEntity = project->findObjectByRay(sceneProject->id, x, y);

                if (selEntity == NULL_ENTITY || lastSelEntity != selEntity) {
                    if (selMesh) {
                        if (selMesh->submeshes[0].material.baseColorTexture != originalTex) {
                            selMesh->submeshes[0].material.baseColorTexture = originalTex;
                            selMesh->submeshes[0].needUpdateTexture = true;
                        }
                        selMesh = nullptr;
                    }
                    if (selUI) {
                        if (selUI->texture != originalTex) {
                            selUI->texture = originalTex;
                            selUI->needUpdateTexture = true;
                        }
                        selUI = nullptr;
                    }
                    if (selText) {
                        if (selText->font != originalFont) {
                            selText->font = originalFont;
                            selText->needReloadAtlas = true;
                            selText->needUpdateText = true;
                        }
                        selText = nullptr;
                    }
                }

                if (selEntity != NULL_ENTITY) {
                    bool accept = false;
                    if (isFont && sceneProject->scene->findComponent<TextComponent>(selEntity)) {
                        accept = true;
                    }
                    if (isImage && (sceneProject->scene->findComponent<MeshComponent>(selEntity) || sceneProject->scene->findComponent<UIComponent>(selEntity))) {
                        accept = true;
                    }

                    if (accept) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                            lastSelEntity = selEntity;

                            if (tempImage != nullptr) {
                                delete tempImage;
                                tempImage = nullptr;
                            }

                            if (TextComponent* text = sceneProject->scene->findComponent<TextComponent>(selEntity)) {
                                if (isFont) {
                                    if (!selText) {
                                        selText = text;
                                        originalFont = text->font;
                                    }
                                    if (text->font != droppedRelativePath) {
                                        text->font = droppedRelativePath;
                                        text->needReloadAtlas = true;
                                        text->needUpdateText = true;
                                    }
                                    if (payload->IsDelivery()) {
                                        std::string propName = "font";
                                        text->font = originalFont;
                                        text->needReloadAtlas = true;
                                        text->needUpdateText = true;

                                        PropertyCmd<std::string>* cmd = new PropertyCmd<std::string>(project, sceneProject->id, selEntity, ComponentType::TextComponent, propName, droppedRelativePath);
                                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);

                                        selText = nullptr;

                                        ImGui::SetWindowFocus();
                                    }
                                }
                            } else if (isImage) {
                                if (MeshComponent* mesh = sceneProject->scene->findComponent<MeshComponent>(selEntity)) {
                                    if (!selMesh) {
                                        selMesh = mesh;
                                        originalTex = mesh->submeshes[0].material.baseColorTexture;
                                    }
                                    Texture newTex(droppedRelativePath);
                                    if (mesh->submeshes[0].material.baseColorTexture != newTex) {
                                        mesh->submeshes[0].material.baseColorTexture = newTex;
                                        mesh->submeshes[0].needUpdateTexture = true;
                                    }
                                    if (payload->IsDelivery()) {
                                        std::string propName = "submeshes[0].material.baseColorTexture";
                                        selMesh->submeshes[0].material.baseColorTexture = originalTex;

                                        PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(project, sceneProject->id, selEntity, ComponentType::MeshComponent, propName, newTex);
                                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);

                                        selMesh = nullptr;

                                        ImGui::SetWindowFocus();
                                    }
                                } else if (UIComponent* ui = sceneProject->scene->findComponent<UIComponent>(selEntity)) {
                                    if (!selUI) {
                                        selUI = ui;
                                        originalTex = ui->texture;
                                    }
                                    Texture newTex(droppedRelativePath);
                                    if (ui->texture != newTex) {
                                        ui->texture = newTex;
                                        ui->needUpdateTexture = true;
                                    }
                                    if (payload->IsDelivery()) {
                                        std::string propName = "texture";
                                        selUI->texture = originalTex;

                                        PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(project, sceneProject->id, selEntity, ComponentType::UIComponent, propName, newTex);
                                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);

                                        selUI = nullptr;

                                        ImGui::SetWindowFocus();
                                    }
                                }
                            }
                        }
                    }
                } else if (sceneProject->sceneType != SceneType::SCENE_3D) {
                    if (isImage) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {

                            if (!tempImage) {
                                tempImage = new Image(sceneProject->scene);
                                tempImage->setTexture(droppedRelativePath);
                                tempImage->setAlpha(0.5f);
                            }
                            Ray ray = sceneProject->sceneRender->getCamera()->screenToRay(x, y);
                            RayReturn rreturn = ray.intersects(Plane(Vector3(0, 0, 1), Vector3(0, 0, 0)));
                            if (rreturn) {
                                tempImage->setPosition(rreturn.point);
                            }
                            if (payload->IsDelivery()) {
                                CreateEntityCmd* cmd = nullptr;

                                if (sceneProject->sceneType == SceneType::SCENE_2D) {
                                    cmd = new CreateEntityCmd(project, sceneProject->id, "Sprite", EntityCreationType::SPRITE);

                                    cmd->addProperty<Vector3>(ComponentType::Transform, "position", rreturn.point);
                                    cmd->addProperty<Texture>(ComponentType::MeshComponent, "submeshes[0].material.baseColorTexture", Texture(droppedRelativePath));
                                    cmd->addProperty<unsigned int>(ComponentType::SpriteComponent, "width", tempImage->getWidth());
                                    cmd->addProperty<unsigned int>(ComponentType::SpriteComponent, "height", tempImage->getHeight());
                                } else {
                                    cmd = new CreateEntityCmd(project, sceneProject->id, "Image", EntityCreationType::IMAGE);

                                    cmd->addProperty<Vector3>(ComponentType::Transform, "position", rreturn.point);
                                    cmd->addProperty<Texture>(ComponentType::UIComponent, "texture", Texture(droppedRelativePath));
                                    cmd->addProperty<unsigned int>(ComponentType::UILayoutComponent, "width", tempImage->getWidth());
                                    cmd->addProperty<unsigned int>(ComponentType::UILayoutComponent, "height", tempImage->getHeight());
                                }

                                CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);

                                delete tempImage;
                                tempImage = nullptr;

                                ImGui::SetWindowFocus();
                            }

                        }
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    } else {
        if (draggingResourceFile) {
            // If user released mouse (ended drag) and we have a temp texture applied, revert it
            if (selMesh) {
                if (selMesh->submeshes[0].material.baseColorTexture != originalTex) {
                    selMesh->submeshes[0].material.baseColorTexture = originalTex;
                    selMesh->submeshes[0].needUpdateTexture = true;
                }
                selMesh = nullptr;
            }
            if (selUI) {
                if (selUI->texture != originalTex) {
                    selUI->texture = originalTex;
                    selUI->needUpdateTexture = true;
                }
                selUI = nullptr;
            }
            if (selText) {
                if (selText->font != originalFont) {
                    selText->font = originalFont;
                    selText->needReloadAtlas = true;
                    selText->needUpdateText = true;
                }
                selText = nullptr;
            }
            draggingResourceFile = false; // Reset flag
            lastSelEntity = NULL_ENTITY;
        }
        if (tempImage != nullptr) {
            delete tempImage;
            tempImage = nullptr;
        }
    }
}

void Editor::SceneWindow::sceneEventHandler(SceneProject* sceneProject) {
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

    // When scene is playing, forward mouse events to Engine
    if (sceneProject->playState == ScenePlayState::PLAYING && isMouseInWindow) {
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        int mods = 0;
        if (io.KeyShift) mods |= S_MODIFIER_SHIFT;
        if (io.KeyCtrl) mods |= S_MODIFIER_CONTROL;
        if (io.KeyAlt) mods |= S_MODIFIER_ALT;
        if (io.KeySuper) mods |= S_MODIFIER_SUPER;

        Engine::systemMouseMove(x, y, mods);

        if (mouseWheel != 0) {
            Engine::systemMouseScroll(0, mouseWheel, mods);
        }

        for (int i = 0; i < 5; i++) {
            if (ImGui::IsMouseClicked(i)) {
                Engine::systemMouseDown(i, x, y, mods);
            }
            if (ImGui::IsMouseReleased(i)) {
                Engine::systemMouseUp(i, x, y, mods);
            }
        }
        return;
    }

    size_t sceneId = sceneProject->id;

    bool disableSelection = 
        sceneProject->sceneRender->getCursorSelected() == CursorSelected::HAND || 
        sceneProject->sceneRender->isAnyGizmoSideSelected() ||
        sceneProject->sceneType != SceneType::SCENE_3D && ImGui::IsKeyDown(ImGuiKey_Space);

    if (isMouseInWindow){

        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            // Selecting and dragging an unselected object at same time (just for 2D object mode)
            GizmoSelected gizmoSelected = sceneProject->sceneRender->getToolsLayer()->getGizmoSelected();
            if (!disableSelection && gizmoSelected == GizmoSelected::OBJECT2D) {
                Entity hitEntity = project->findObjectByRay(sceneId, x, y);
                if (hitEntity != NULL_ENTITY) {
                    bool alreadySelected = project->isSelectedEntity(sceneId, hitEntity);
                    if (!alreadySelected || io.KeyShift) {
                        bool changed = project->selectObjectByRay(sceneId, x, y, io.KeyShift);
                        if (changed) {
                            sceneProject->sceneRender->update(project->getSelectedEntities(sceneId), project->getEntities(sceneId), sceneProject->mainCamera);
                            sceneProject->sceneRender->mouseHoverEvent(x, y);
                        }
                    }
                }
            }
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
            }
            if (mouseLeftDraggedInside){
                sceneProject->sceneRender->mouseDragEvent(x, y, mouseLeftStartPos.x, mouseLeftStartPos.y, project, sceneId, project->getSelectedEntities(sceneId), disableSelection);
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
            if (!mouseLeftDraggedInside && mouseLeftDown && !disableSelection){
                project->selectObjectByRay(sceneId, x, y, io.KeyShift);
            }
            mouseLeftDown = false;
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        float x = mousePos.x - windowPos.x;
        float y = mousePos.y - windowPos.y;

        if (mouseLeftDraggedInside && !disableSelection){
            Vector2 clickStartPos = Vector2((2 * mouseLeftStartPos.x / width[sceneId]) - 1, -((2 * mouseLeftStartPos.y / height[sceneId]) - 1));
            Vector2 clickEndPos = Vector2((2 * mouseLeftDragPos.x / width[sceneId]) - 1, -((2 * mouseLeftDragPos.y / height[sceneId]) - 1));
            project->selectObjectsByRect(sceneId, clickStartPos, clickEndPos);
        }

        sceneProject->sceneRender->mouseReleaseEvent(x, y);

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

    Camera* camera = sceneProject->sceneRender->getCamera();

    bool walkingMode = false;

    if (sceneProject->sceneType == SceneType::SCENE_3D){

        float distanceFromTarget = camera->getDistanceFromTarget();

        // Scale factor: movements should be proportional to distance
        // Base movement rate is maintained at around distanceFromTarget = 10
        float distanceScaleFactor = std::max(0.1f, distanceFromTarget / 10.0f);

        // Check for mouse clicks
        if (draggingMouse[sceneId] && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                walkingMode = true;

                camera->rotateView(-0.1 * mouseDelta.x);
                camera->elevateView(-0.1 * mouseDelta.y);

                float minSpeed = 0.5;
                float maxSpeed = 1000;
                float speedOffset = 10.0;

                walkSpeed[sceneId] += mouseWheel;
                if (walkSpeed[sceneId] <= -speedOffset) {
                    walkSpeed[sceneId] = -speedOffset + minSpeed;
                }
                if (walkSpeed[sceneId] > maxSpeed) {
                    walkSpeed[sceneId] = maxSpeed;
                }

                // Apply distanceScaleFactor to walking speed
                float finalSpeed = 0.02 * (speedOffset + walkSpeed[sceneId]) * distanceScaleFactor;

                if (ImGui::IsKeyDown(ImGuiKey_W)) {
                    camera->slideForward(finalSpeed);
                }
                if (ImGui::IsKeyDown(ImGuiKey_S)) {
                    camera->slideForward(-finalSpeed);
                }
                if (ImGui::IsKeyDown(ImGuiKey_A)) {
                    camera->slide(-finalSpeed);
                }
                if (ImGui::IsKeyDown(ImGuiKey_D)) {
                    camera->slide(finalSpeed);
                }
                if (ImGui::IsKeyDown(ImGuiKey_E)) {
                    camera->slideUp(finalSpeed);
                }
                if (ImGui::IsKeyDown(ImGuiKey_Q)) {
                    camera->slideUp(-finalSpeed);
                }

                io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
                Backend::disableMouseCursor();
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
                    camera->slide(-0.01 * mouseDelta.x * distanceScaleFactor);
                    camera->slideUp(0.01 * mouseDelta.y * distanceScaleFactor);
                } else if (ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
                    camera->zoom(-0.1 * mouseDelta.y);
                } else {
                    camera->rotatePosition(-0.1 * mouseDelta.x);
                    camera->elevatePosition(0.1 * mouseDelta.y);
                }
            }
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && sceneProject->sceneRender->getCursorSelected() == CursorSelected::HAND) {
            camera->slide(-0.01 * mouseDelta.x * distanceScaleFactor);
            camera->slideUp(0.01 * mouseDelta.y * distanceScaleFactor);
        }

        // The zoom speed itself can remain constant
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (isMouseInWindow && mouseWheel != 0.0f) {
                camera->zoom(2.0 * mouseWheel);
            }
        }

    }else{

        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || 
                (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsKeyDown(ImGuiKey_Space)) ||
                (ImGui::IsMouseDown(ImGuiMouseButton_Left) && sceneProject->sceneRender->getCursorSelected() == CursorSelected::HAND)){

            SceneRender2D* sceneRender2D = static_cast<SceneRender2D*>(sceneProject->sceneRender);
            float currentZoom = sceneRender2D->getZoom();

            float slideX = -currentZoom * mouseDelta.x;
            float slideY = -currentZoom * mouseDelta.y;

            if (sceneProject->sceneType == SceneType::SCENE_2D){
                slideY = -slideY;
            }

            camera->slide(slideX);
            camera->slideUp(slideY);
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)){
            if (isMouseInWindow && mouseWheel != 0.0f){
                float zoomFactor = 1.0f - (0.1f * mouseWheel);

                float mouseX = mousePos.x - windowPos.x;
                float mouseY = mousePos.y - windowPos.y;

                if (sceneProject->sceneType == SceneType::SCENE_2D){
                    mouseY = height[sceneProject->id] - mouseY;
                }

                SceneRender2D* sceneRender2D = static_cast<SceneRender2D*>(sceneProject->sceneRender);
                sceneRender2D->zoomAtPosition(width[sceneProject->id], height[sceneProject->id], Vector2(mouseX, mouseY), zoomFactor);
            }
        }

    }

    if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused()){
        if (project->getSelectedSceneId() == sceneId){
            if (!walkingMode){

                if (sceneProject->sceneType != SceneType::SCENE_UI){
                    if (ImGui::IsKeyPressed(ImGuiKey_W)) {
                        sceneProject->sceneRender->getToolsLayer()->enableTranslateGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                        sceneProject->sceneRender->getToolsLayer()->enableRotateGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                        sceneProject->sceneRender->getToolsLayer()->enableScaleGizmo();
                    }

                    if (ImGui::IsKeyPressed(ImGuiKey_T)){
                        sceneProject->sceneRender->changeUseGlobalTransform();
                    }
                }
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

    int openedScenesCount = 0;
    for (const auto& s : project->getScenes()) {
        if (s.opened) openedScenesCount++;
    }

    // Iterate through all scenes in the project
    for (auto& sceneProject : project->getScenes()) {
        if (!sceneProject.opened) continue;

        // Disable close button if this is the only open scene
        bool canClose = openedScenesCount > 1;
        bool isOpen = true;

        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin(getWindowTitle(sceneProject).c_str(), canClose ? &isOpen : nullptr)) {
            sceneProject.isVisible = true;

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                windowFocused = true;
                project->setSelectedSceneId(sceneProject.id);
            }

            bool isPlaying = (sceneProject.playState == ScenePlayState::PLAYING);
            bool isPaused = (sceneProject.playState == ScenePlayState::PAUSED);
            bool isStopped = (sceneProject.playState == ScenePlayState::STOPPED);
            bool isCancelling = (sceneProject.playState == ScenePlayState::CANCELLING);

            // Play button - disabled when already playing
            ImGui::BeginDisabled(isPlaying || isCancelling || (isStopped && project->isAnyScenePlaying()));
            if (ImGui::Button(ICON_FA_PLAY " Play")) {
                if (!isPaused) {
                    project->start(sceneProject.id);
                }else{
                    project->resume(sceneProject.id);
                }
            }
            ImGui::EndDisabled();

            // Pause/Resume button - disabled when stopped
            ImGui::BeginDisabled(isStopped || isPaused || isCancelling);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PAUSE " Pause")) {
                project->pause(sceneProject.id);
            }
            ImGui::EndDisabled();

            // Stop button - disabled when stopped
            ImGui::BeginDisabled(isStopped || isCancelling);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_STOP " Stop")) {
                project->stop(sceneProject.id);
            }
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));
            ImGui::SameLine(0, 10);

            CursorSelected cursorSelected = sceneProject.sceneRender->getCursorSelected();

            ImGui::BeginDisabled(cursorSelected == CursorSelected::POINTER);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_POINTER)) {
                sceneProject.sceneRender->enableCursorPointer();
            }
            ImGui::SetItemTooltip("Select mode");
            ImGui::EndDisabled();

            ImGui::BeginDisabled(cursorSelected == CursorSelected::HAND);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_HAND)) {
                sceneProject.sceneRender->enableCursorHand();
            }
            ImGui::SetItemTooltip("Pan view");
            ImGui::EndDisabled();

            ImGui::SameLine(0, 10);
            ImGui::Dummy(ImVec2(1, 20));

            GizmoSelected gizmoSelected = sceneProject.sceneRender->getToolsLayer()->getGizmoSelected();
            bool multipleEntitiesSelected = sceneProject.sceneRender->isMultipleEntitesSelected();

            if (sceneProject.sceneType != SceneType::SCENE_UI){
                ImGui::SameLine(0, 10);

                if (sceneProject.sceneType != SceneType::SCENE_3D){
                    ImGui::BeginDisabled(gizmoSelected == GizmoSelected::OBJECT2D);
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_EXPAND)) {
                        sceneProject.sceneRender->getToolsLayer()->enableObject2DGizmo();
                    }
                    ImGui::SetItemTooltip("2D Gizmo (Q)");
                    ImGui::EndDisabled();
                }

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

                ImGui::BeginDisabled(useGlobalTransform || gizmoSelected == GizmoSelected::OBJECT2D || multipleEntitiesSelected);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_GLOBE)) {
                    sceneProject.sceneRender->setUseGlobalTransform(true);
                }
                ImGui::SetItemTooltip("World transform (T)");
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!useGlobalTransform || gizmoSelected == GizmoSelected::OBJECT2D || multipleEntitiesSelected);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_LOCATION_DOT)) {
                    sceneProject.sceneRender->setUseGlobalTransform(false);
                }
                ImGui::SetItemTooltip("Local transform (T)");
                ImGui::EndDisabled();

                ImGui::SameLine(0, 10);
                ImGui::Dummy(ImVec2(1, 20));
            }

            ImGui::BeginChild(("Canvas" + std::to_string(sceneProject.id)).c_str());
            {
                int widthNew = ImGui::GetContentRegionAvail().x;
                int heightNew = ImGui::GetContentRegionAvail().y;

                if (widthNew != width[sceneProject.id] || heightNew != height[sceneProject.id]) {
                    width[sceneProject.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneProject.id] = ImGui::GetContentRegionAvail().y;

                    sceneProject.needUpdateRender = true;
                }

                ImGui::Image((ImTextureID)(intptr_t)sceneProject.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

                if (sceneProject.playState == ScenePlayState::STOPPED){
                    if (ImGui::IsWindowHovered()) {
                        CursorSelected cursorSelected = sceneProject.sceneRender->getCursorSelected();
                        if (cursorSelected == CursorSelected::HAND) {
                            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                        } else {
                            Gizmo2DSideSelected side = sceneProject.sceneRender->getToolsLayer()->getGizmo2DSideSelected();
                            if (side == Gizmo2DSideSelected::NX || side == Gizmo2DSideSelected::PX) {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                            } else if (side == Gizmo2DSideSelected::NY || side == Gizmo2DSideSelected::PY) {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                            } else if (side == Gizmo2DSideSelected::NX_NY || side == Gizmo2DSideSelected::PX_PY) {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                            } else if (side == Gizmo2DSideSelected::NX_PY || side == Gizmo2DSideSelected::PX_NY) {
                                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                            }
                        }
                    }

                    handleResourceFileDragDrop(&sceneProject);
                }

                sceneEventHandler(&sceneProject);
            }
            ImGui::EndChild();
        }else{
            sceneProject.isVisible = false;
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
