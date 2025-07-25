#include "SceneWindow.h"

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
    std::string icon;
    if (sceneProject.sceneType == SceneType::SCENE_3D){
        icon = ICON_FA_CUBES + std::string("  ");
    }else if (sceneProject.sceneType == SceneType::SCENE_2D){
        icon = ICON_FA_CUBES_STACKED + std::string("  ");
    }else if (sceneProject.sceneType == SceneType::SCENE_UI){
        icon = ICON_FA_WINDOW_RESTORE + std::string("  ");
    }
    return icon + sceneProject.name + (sceneProject.isModified ? " *" : "") + "###Scene" + std::to_string(sceneProject.id);
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

    SceneProject* sceneProject = project->getScene(sceneId);

    bool disableSelection = 
        sceneProject->sceneRender->getCursorSelected() == CursorSelected::HAND || 
        sceneProject->sceneRender->isAnyGizmoSideSelected() ||
        sceneProject->sceneType != SceneType::SCENE_3D && ImGui::IsKeyDown(ImGuiKey_Space);

    if (isMouseInWindow){

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
            }
            if (mouseLeftDraggedInside){
                sceneProject->sceneRender->mouseDragEvent(x, y, mouseLeftStartPos.x, mouseLeftStartPos.y, sceneId, sceneProject, project->getSelectedEntities(sceneId), disableSelection);
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
            ImGui::SameLine(0, 10);

            GizmoSelected gizmoSelected = sceneProject.sceneRender->getToolsLayer()->getGizmoSelected();
            bool multipleEntitiesSelected = sceneProject.sceneRender->isMultipleEntitesSelected();

            if (sceneProject.sceneType != SceneType::SCENE_UI){

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
                ImGui::SameLine(0, 10);

            }

            if (ImGui::Button(ICON_FA_GEAR)) {
                ImGui::OpenPopup("scenesettings");
            }
            ImGui::SetNextWindowSizeConstraints(ImVec2(250.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
            if (ImGui::BeginPopup("scenesettings")) {
                ImGui::Text("Scene settings");
                ImGui::Separator();
                ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame;

                // Start a table for properties
                if (ImGui::BeginTable("scene_settings_table", 2, tableFlags)) {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Background").x);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                    drawSceneProperty<Vector4>(&sceneProject, "background_color", "Background", ScenePropertyType::COLOR_RGBA);
                    drawSceneProperty<LightState>(&sceneProject, "light_state", "Lights", ScenePropertyType::COMBO);

                    ImGui::EndTable();
                }

                LightState currentLightState = Supernova::Editor::Catalog::getSceneProperty<LightState>(sceneProject.scene, "light_state");

                if (currentLightState != LightState::OFF) {
                    ImGui::SeparatorText("Global Illumination");

                    if (ImGui::BeginTable("scene_globalillum_table", 2, tableFlags)) {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Intensity").x);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                        drawSceneProperty<Vector3>(&sceneProject, "global_illumination_color", "Color", ScenePropertyType::COLOR_RGB);
                        drawSceneProperty<float> (&sceneProject, "global_illumination_intensity", "Intensity", ScenePropertyType::SLIDER_FLOAT, 0.0f, 1.0f);

                        ImGui::EndTable();
                    }

                    ImGui::SeparatorText("Shadows");

                    if (ImGui::BeginTable("scene_shadow_settings_table", 2, tableFlags)) {
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Enable PCF").x);
                        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                        drawSceneProperty<bool>  (&sceneProject, "shadows_pcf", "Enable PCF", ScenePropertyType::CHECKBOX);

                        ImGui::EndTable();
                    }
                }

                ImGui::EndPopup();
            }

            ImGui::BeginChild(("Canvas" + std::to_string(sceneProject.id)).c_str());
            {
                if (ImGui::IsWindowHovered()) {
                    CursorSelected cursorSelected = sceneProject.sceneRender->getCursorSelected();
                    if (cursorSelected == CursorSelected::HAND) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                    }
                }

                sceneEventHandler(project, sceneProject.id);

                int widthNew = ImGui::GetContentRegionAvail().x;
                int heightNew = ImGui::GetContentRegionAvail().y;

                if (widthNew != width[sceneProject.id] || heightNew != height[sceneProject.id]) {
                    width[sceneProject.id] = ImGui::GetContentRegionAvail().x;
                    height[sceneProject.id] = ImGui::GetContentRegionAvail().y;

                    sceneProject.needUpdateRender = true;
                }

                ImGui::Image((ImTextureID)(intptr_t)sceneProject.sceneRender->getTexture().getGLHandler(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
                
                static bool draggingResourceFile = false;
                static Image* tempImage = nullptr;
                static MeshComponent* selMesh = nullptr;
                static UIComponent* selUI = nullptr;
                static Texture originalTex;
                static Entity lastSelEntity = NULL_ENTITY;

                if (ImGui::BeginDragDropTarget()) {
                    draggingResourceFile = true;
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImGuiIO& io = ImGui::GetIO();
                    ImVec2 mousePos = io.MousePos;
                    float x = mousePos.x - windowPos.x;
                    float y = mousePos.y - windowPos.y;
                    Entity selEntity = project->findObjectByRay(sceneProject.id, x, y);

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
                    }

                    if (selEntity != NULL_ENTITY) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                            lastSelEntity = selEntity;
                            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
                            if (receivedStrings.size() > 0){
                                if (tempImage != nullptr) {
                                    delete tempImage;
                                    tempImage = nullptr;
                                }
                                if (MeshComponent* mesh = sceneProject.scene->findComponent<MeshComponent>(selEntity)) {
                                    if (!selMesh) {
                                        selMesh = mesh;
                                        originalTex = mesh->submeshes[0].material.baseColorTexture;
                                    }
                                    Texture newTex(receivedStrings[0]);
                                    if (mesh->submeshes[0].material.baseColorTexture != newTex) {
                                        mesh->submeshes[0].material.baseColorTexture = newTex;
                                        mesh->submeshes[0].needUpdateTexture = true;
                                    }
                                    if (payload->IsDelivery()) {
                                        std::string propName = "submeshes[0].material.basecolortexture";
                                        selMesh->submeshes[0].material.baseColorTexture = originalTex;

                                        PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(&sceneProject, selEntity, ComponentType::MeshComponent, propName, UpdateFlags_Mesh_Texture, newTex);
                                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);
                                        cmd->commit();

                                        selMesh = nullptr;

                                        ImGui::SetWindowFocus();
                                    }
                                }else if (UIComponent* ui = sceneProject.scene->findComponent<UIComponent>(selEntity)){
                                    if (!selUI) {
                                        selUI = ui;
                                        originalTex = ui->texture;
                                    }
                                    Texture newTex(receivedStrings[0]);
                                    if (ui->texture != newTex) {
                                        ui->texture = newTex;
                                        ui->needUpdateTexture = true;
                                    }
                                    if (payload->IsDelivery()) {
                                        std::string propName = "texture";
                                        selUI->texture = originalTex;

                                        PropertyCmd<Texture>* cmd = new PropertyCmd<Texture>(&sceneProject, selEntity, ComponentType::UIComponent, propName, UpdateFlags_UI_Texture, newTex);
                                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);
                                        cmd->commit();

                                        selUI = nullptr;

                                        ImGui::SetWindowFocus();
                                    }
                                }
                            }
                        }
                    }else if (sceneProject.sceneType != SceneType::SCENE_3D){
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files", ImGuiDragDropFlags_AcceptBeforeDelivery)) {
                            std::vector<std::string> receivedStrings = Editor::Util::getStringsFromPayload(payload);
                            if (receivedStrings.size() > 0){
                                if (!tempImage){
                                    tempImage = new Image(sceneProject.scene);
                                    tempImage->setTexture(receivedStrings[0]);
                                    tempImage->setAlpha(0.5f);
                                }
                                Ray ray = sceneProject.sceneRender->getCamera()->screenToRay(x, y);
                                RayReturn rreturn = ray.intersects(Plane(Vector3(0, 0, 1), Vector3(0, 0, 0)));
                                if (rreturn){
                                    tempImage->setPosition(rreturn.point);
                                }
                                if (payload->IsDelivery()) {
                                    CreateEntityCmd* cmd = nullptr;

                                    if (sceneProject.sceneType == SceneType::SCENE_2D){
                                        cmd = new CreateEntityCmd(project, sceneProject.id, "Sprite", EntityCreationType::SPRITE);

                                        cmd->addProperty<Vector3>(ComponentType::Transform, "position", rreturn.point);
                                        cmd->addProperty<Texture>(ComponentType::MeshComponent, "submeshes[0].material.basecolortexture", Texture(receivedStrings[0]));
                                        cmd->addProperty<unsigned int>(ComponentType::SpriteComponent, "width", tempImage->getWidth());
                                        cmd->addProperty<unsigned int>(ComponentType::SpriteComponent, "height", tempImage->getHeight());
                                    }else{
                                        cmd = new CreateEntityCmd(project, sceneProject.id, "Image", EntityCreationType::IMAGE);

                                        cmd->addProperty<Vector3>(ComponentType::Transform, "position", rreturn.point);
                                        cmd->addProperty<Texture>(ComponentType::UIComponent, "texture", Texture(receivedStrings[0]));
                                        cmd->addProperty<unsigned int>(ComponentType::UILayoutComponent, "width", tempImage->getWidth());
                                        cmd->addProperty<unsigned int>(ComponentType::UILayoutComponent, "height", tempImage->getHeight());
                                    }

                                    CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(cmd);
                                    cmd->commit();

                                    delete tempImage;
                                    tempImage = nullptr;

                                    ImGui::SetWindowFocus();
                                }
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }else{
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
                        draggingResourceFile = false; // Reset flag
                        lastSelEntity = NULL_ENTITY;
                    }
                    if (tempImage != nullptr) {
                        delete tempImage;
                        tempImage = nullptr;
                    }
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