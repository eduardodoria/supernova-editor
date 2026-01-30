#include "Structure.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/MoveEntityOrderCmd.h"
#include "command/type/AddChildSceneCmd.h"
#include "command/type/RemoveChildSceneCmd.h"
#include "command/type/CreateEntityCmd.h"
#include "command/type/EntityNameCmd.h"
#include "command/type/SceneNameCmd.h"
#include "command/type/DeleteEntityCmd.h"
#include "command/type/ImportSharedEntityCmd.h"
#include "command/type/MakeEntityLocalCmd.h"
#include "command/type/MakeEntitySharedCmd.h"
#include "command/type/SetMainCameraCmd.h"
#include "util/EntityPayload.h"
#include "util/UIUtils.h"
#include "Out.h"
#include "Stream.h"
#include <algorithm>
#include <cctype>
#include <filesystem>

using namespace Supernova;

Editor::Structure::Structure(Project* project, SceneWindow* sceneWindow){
    this->project = project;
    this->sceneWindow = sceneWindow;
    this->openParent = NULL_ENTITY;
}

void Editor::Structure::showNewEntityMenu(bool isScene, Entity parent, bool addToShared){
    if (isScene){
        parent = NULL_ENTITY;

        if (ImGui::MenuItem(ICON_FA_CIRCLE_DOT"  Empty entity")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Entity", addToShared));
        }

        ImGui::Separator();
    }

    if (ImGui::MenuItem(ICON_FA_SITEMAP"  Empty object")){
        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Object", EntityCreationType::OBJECT, parent, addToShared));
        openParent = parent;
    }

    if (ImGui::MenuItem(ICON_FA_CLOUD"  Sky")){
        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Sky", EntityCreationType::SKY, parent, addToShared));
        openParent = parent;
    }

    if (ImGui::MenuItem(ICON_FA_VIDEO"  Camera")){
        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Camera", EntityCreationType::CAMERA, parent, addToShared));
        openParent = parent;
    }

    if (ImGui::BeginMenu(ICON_FA_CUBE"  Basic shape")){
        if (ImGui::MenuItem(ICON_FA_CUBE"  Box")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Box", EntityCreationType::BOX, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Plane")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Plane", EntityCreationType::PLANE, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Sphere")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Sphere", EntityCreationType::SPHERE, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Cylinder")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Cylinder", EntityCreationType::CYLINDER, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Capsule")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Capsule", EntityCreationType::CAPSULE, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Torus")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Torus", EntityCreationType::TORUS, parent, addToShared));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_CUBES_STACKED"  2D")){
        if (ImGui::MenuItem(ICON_FA_IMAGE"  Sprite")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Sprite", EntityCreationType::SPRITE, parent, addToShared));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_WINDOW_RESTORE"  UI")){
        if (ImGui::MenuItem(ICON_FA_IMAGE"  Image")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Image", EntityCreationType::IMAGE, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_FONT"  Text")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Text", EntityCreationType::TEXT, parent, addToShared));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_LIGHTBULB"  Light")){
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Point")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Point Light", EntityCreationType::POINT_LIGHT, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Directional")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Directional Light", EntityCreationType::DIRECTIONAL_LIGHT, parent, addToShared));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Spot")){
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new CreateEntityCmd(project, project->getSelectedSceneId(), "Spot Light", EntityCreationType::SPOT_LIGHT, parent, addToShared));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem(ICON_FA_PERSON_RUNNING"  Model")){
        // Action for Item 2
    }

    ImGui::EndMenu();
}

void Editor::Structure::showIconMenu(){
    if (ImGui::Button(ICON_FA_PLUS)) {
        ImGui::OpenPopup("NewObjectMenu");
    }

    ImGui::SameLine();

    UIUtils::searchInput("##structure_search", "", searchBuffer, sizeof(searchBuffer), false, &matchCase);

    if (ImGui::BeginPopup("NewObjectMenu"))
    {
        ImGui::Text("New scene:");
        ImVec2 buttonSize = ImVec2(ImGui::GetFontSize() * 8, ImGui::GetFontSize() * 2);
        if (ImGui::Button(ICON_FA_CUBES "  3D Scene", buttonSize)) {
            uint32_t sceneid = project->createNewScene("New Scene", SceneType::SCENE_3D);
        }
        //ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CUBES_STACKED "  2D Scene", buttonSize)) {
            uint32_t sceneid = project->createNewScene("New Scene", SceneType::SCENE_2D);
        }
        //ImGui::SameLine();
        if (ImGui::Button(ICON_FA_WINDOW_RESTORE "  UI Scene", buttonSize)) {
            uint32_t sceneid = project->createNewScene("New Scene", SceneType::SCENE_UI);
        }
        ImGui::Separator();
        if (ImGui::BeginMenu(ICON_FA_CIRCLE_DOT"  Create entity"))
        {
            showNewEntityMenu(true, NULL_ENTITY, false);
        }

        ImGui::EndPopup();
    }
}

void Editor::Structure::drawInsertionMarker(const ImVec2& p1, const ImVec2& p2) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 col = ImGui::GetColorU32(ImGuiCol_DragDropTarget);
    float thickness = 2.0f;
    draw_list->AddLine(p1, p2, col, thickness);
}

std::string Editor::Structure::getObjectIcon(Signature signature, Scene* scene){
    if (signature.test(scene->getComponentId<ModelComponent>())){
        return ICON_FA_PERSON_WALKING;
    }else if (signature.test(scene->getComponentId<MeshComponent>())){
        return ICON_FA_CUBE;
    }else if (signature.test(scene->getComponentId<SkyComponent>())){
        return ICON_FA_CLOUD;
    }else if (signature.test(scene->getComponentId<UIComponent>())){
        return ICON_FA_IMAGE;
    }else if (signature.test(scene->getComponentId<LightComponent>())){
        return ICON_FA_LIGHTBULB;
    }else if (signature.test(scene->getComponentId<CameraComponent>())){
        return ICON_FA_VIDEO;
    }else if (signature.test(scene->getComponentId<Transform>())){
        return ICON_FA_SITEMAP;
    }

    return ICON_FA_CIRCLE_DOT;
}

Editor::TreeNode* Editor::Structure::findNode(Editor::TreeNode* root, Entity entity){
     for (int i = 0; i < root->children.size(); i++){
        Editor::TreeNode* node = &root->children[i];
        if (!node->isScene && node->id == entity){
            return node;
        }

        if (Editor::TreeNode* child = findNode(node, entity)){
            return child;
        }
     }

     return nullptr;
}

void Editor::Structure::handleEntityFilesDrop(const std::vector<std::string>& filePaths, Entity parent) {
    for (const std::string& filePath : filePaths) {
        std::filesystem::path path(filePath);

        // Check if it's an entity file
        if (path.extension() == ".entity") {
            // Import the shared entity into the current scene using command
            std::filesystem::path relativePath = std::filesystem::relative(path, project->getProjectPath());

            ImportSharedEntityCmd* importCmd = new ImportSharedEntityCmd(project, project->getSelectedSceneId(), relativePath, parent, true);
            CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(importCmd);

            std::vector<Entity> newEntities = importCmd->getImportedEntities();

            if (newEntities.size() > 0) {
                Out::info("Successfully imported entity from: %s", path.string().c_str());
            } else {
                Out::warning("Failed to import entity from: %s (might already exist in scene)", path.string().c_str());
            }
        }
    }
}

void Editor::Structure::handleSceneFilesDropAsChildScenes(const std::vector<std::string>& filePaths, uint32_t ownerSceneId) {
    if (ownerSceneId == NULL_PROJECT_SCENE) {
        return;
    }

    auto normalizeDroppedPath = [this](const std::filesystem::path& p) -> std::filesystem::path {
        if (p.empty()) {
            return p;
        }
        if (p.is_relative()) {
            return (project->getProjectPath() / p).lexically_normal();
        }
        return p.lexically_normal();
    };

    auto pathsReferToSameFile = [](const std::filesystem::path& a, const std::filesystem::path& b) -> bool {
        std::error_code ec;
        if (std::filesystem::exists(a, ec) && std::filesystem::exists(b, ec)) {
            if (std::filesystem::equivalent(a, b, ec) && !ec) {
                return true;
            }
        }
        return a.lexically_normal() == b.lexically_normal();
    };

    for (const std::string& filePath : filePaths) {
        std::filesystem::path droppedPath = normalizeDroppedPath(std::filesystem::path(filePath));
        if (droppedPath.extension() != ".scene") {
            continue;
        }

        uint32_t droppedSceneId = NULL_PROJECT_SCENE;
        for (const auto& scene : project->getScenes()) {
            if (!scene.filepath.empty() && pathsReferToSameFile(scene.filepath, droppedPath)) {
                droppedSceneId = scene.id;
                break;
            }
        }

        if (droppedSceneId == NULL_PROJECT_SCENE) {
            Out::warning("Dropped scene is not part of the current project: %s", droppedPath.string().c_str());
            continue;
        }

        CommandHandle::get(ownerSceneId)->addCommand(new AddChildSceneCmd(project, ownerSceneId, droppedSceneId));
    }
}

void Editor::Structure::showAddChildSceneMenu() {
    if (ImGui::BeginMenu(ICON_FA_FOLDER_TREE "  Add child scene")) {
        uint32_t currentSceneId = project->getSelectedSceneId();
        const auto& scenes = project->getScenes();

        bool hasAvailableScenes = false;
        for (const auto& scene : scenes) {
            // Skip current scene and already added child scenes
            if (scene.id == currentSceneId) {
                continue;
            }
            if (project->hasChildScene(currentSceneId, scene.id)) {
                continue;
            }

            hasAvailableScenes = true;
            std::string menuLabel = scene.name + " (ID: " + std::to_string(scene.id) + ")";
            if (ImGui::MenuItem(menuLabel.c_str())) {
                CommandHandle::get(currentSceneId)->addCommand(new AddChildSceneCmd(project, currentSceneId, scene.id));
            }
        }

        if (!hasAvailableScenes) {
            ImGui::TextDisabled("No available scenes");
        }

        ImGui::EndMenu();
    }
}

bool Editor::Structure::nodeMatchesSearch(const TreeNode& node, const std::string& searchLower) {
    std::string nodeName = node.name;
    std::string searchStr = searchLower;

    if (!matchCase) {
        // Convert both to lowercase for case-insensitive search
        std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(), ::tolower);
        std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);
    }

    // Check if the node name contains the search string
    return nodeName.find(searchStr) != std::string::npos;
}

bool Editor::Structure::hasMatchingDescendant(const TreeNode& node, const std::string& searchLower) {
    // Check if any child matches
    for (const auto& child : node.children) {
        if (nodeMatchesSearch(child, searchLower)) {
            return true;
        }
        // Recursively check descendants
        if (hasMatchingDescendant(child, searchLower)) {
            return true;
        }
    }
    return false;
}

void Editor::Structure::markMatchingNodes(TreeNode& node, const std::string& searchLower) {
    node.matchesSearch = nodeMatchesSearch(node, searchLower);
    node.hasMatchingDescendant = false;

    for (auto& child : node.children) {
        markMatchingNodes(child, searchLower);
        if (child.matchesSearch || child.hasMatchingDescendant) {
            node.hasMatchingDescendant = true;
        }
    }
}

void Editor::Structure::showTreeNode(Editor::TreeNode& node) {
    // Skip nodes that don't match search and don't have matching descendants
    bool hasSearch = strlen(searchBuffer) > 0;
    if (hasSearch && !node.matchesSearch && !node.hasMatchingDescendant) {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (std::find(selectedScenes.begin(), selectedScenes.end(), &node) != selectedScenes.end()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!node.isScene && !node.isChildScene && project->isSelectedEntity(project->getSelectedSceneId(), node.id)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Auto-expand nodes when searching to show matches
    if (hasSearch && node.hasMatchingDescendant) {
        ImGui::SetNextItemOpen(true);
    } else if (!hasSearch) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    if(!node.isScene && openParent == node.id) {
        ImGui::SetNextItemOpen(true);
        openParent = NULL_ENTITY;
    }

    // Check if entity is shared and get shared group info
    node.isShared = false;
    std::filesystem::path sharedFilepath;

    if (!node.isScene && !node.isChildScene) {
        sharedFilepath = project->findGroupPathFor(project->getSelectedSceneId(), node.id);
        node.isShared = !sharedFilepath.empty();

        if (node.parent != NULL_ENTITY) {
            node.isParentShared = project->isEntityShared(project->getSelectedSceneId(), node.parent);
        }
    }

    // Push colors for visual feedback
    bool pushedHighlightColor = false;

    // Highlight matching nodes when searching
    if (hasSearch && node.matchesSearch) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.5f, 1.0f)); // Yellow for search matches
        pushedHighlightColor = true;
    } else if (node.isChildScene) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 0.6f, 1.0f)); // Light green for child scenes
        pushedHighlightColor = true;
    } else if (node.isShared) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f)); // Light blue for shared
        pushedHighlightColor = true;
    }

    std::string label = node.icon + "  " + node.name;
    if (node.isMainCamera){
        label += "  " ICON_FA_ASTERISK;
    }
    label += "###" + getNodeImGuiId(node);

    bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

    // Pop color if we pushed it
    if (pushedHighlightColor) {
        ImGui::PopStyleColor();
    }

    if (node.isScene){
        ImGui::SetItemTooltip("Id: %u", node.id);
    }else if (node.isChildScene){
        ImGui::SetItemTooltip("Child Scene\nId: %u", node.childSceneId);
    }else{
        if (node.isShared) {
            ImGui::SetItemTooltip("Entity: %u (Shared)\nPath: %s", node.id, sharedFilepath.string().c_str());
        } else {
            ImGui::SetItemTooltip("Entity: %u", node.id);
        }
    }

    if (!node.isChildScene && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        // Add entity drag drop payload for dragging to resources
        if (!node.isScene) {
            SceneProject* sceneProject = project->getSelectedScene();
            YAML::Node entityData = Stream::encodeEntity(node.id, sceneProject->scene, project, sceneProject);
            std::string yamlString = YAML::Dump(entityData);

            size_t yamlSize = yamlString.size();
            size_t payloadSize = sizeof(EntityPayload) + yamlSize;
            std::vector<char> payloadData(payloadSize);

            EntityPayload* payload = reinterpret_cast<EntityPayload*>(payloadData.data());
            payload->entity = node.id;
            payload->parent = node.parent;
            payload->order = node.order;
            payload->hasTransform = node.hasTransform;
            memcpy(payloadData.data() + sizeof(EntityPayload), yamlString.data(), yamlSize);

            ImGui::SetDragDropPayload("entity", payloadData.data(), payloadSize);
        }

        ImGui::Text("Moving %s", node.name.c_str());
        ImGui::EndDragDropSource();
    }

    bool insertBefore = false;
    bool insertAfter = false;

    if (!node.isChildScene && ImGui::BeginDragDropTarget()) {
        bool allowEntityDragDrop = false;
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        Entity sourceEntity = 0;
        Entity sourceParent = 0;
        size_t sourceOrder = 0;
        bool sourceHasTransform = false;
        if (payload && payload->IsDataType("entity")) {
            if (payload->DataSize >= sizeof(EntityPayload)) {
                allowEntityDragDrop = true;
                const EntityPayload* p = reinterpret_cast<const EntityPayload*>(payload->Data);
                sourceEntity = p->entity;
                sourceParent = p->parent;
                sourceOrder = p->order;
                sourceHasTransform = p->hasTransform;
            }

            if (sourceHasTransform != node.hasTransform) {
                allowEntityDragDrop = false;
            }
        }

        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        if (!node.isScene && node.hasTransform){
            insertBefore = (mousePos.y - itemMin.y) < (itemMax.y - itemMin.y) * 0.2f;
            insertAfter = (mousePos.y - itemMin.y) > (itemMax.y - itemMin.y) * 0.8f;
        }else{
            insertBefore = (mousePos.y - itemMin.y) < (itemMax.y - itemMin.y) * 0.5f;
            insertAfter = (mousePos.y - itemMin.y) >= (itemMax.y - itemMin.y) * 0.5f;
        }

        if (node.parent == sourceParent){
            if (node.order == (sourceOrder+1) && insertBefore){
                allowEntityDragDrop = false;
            }
            if (node.order == (sourceOrder-1) && insertAfter){
                allowEntityDragDrop = false;
            }
        }

        if (allowEntityDragDrop){
            ImGuiDragDropFlags flags = 0;
            //ImGuiDragDropFlags flags = ImGuiDragDropFlags_AcceptBeforeDelivery;

            if (insertBefore || insertAfter){
                flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
            }

            if (ImGui::AcceptDragDropPayload("entity", flags)) {

                if (!node.isScene && payload->IsDelivery()){
                    InsertionType type;
                    if (insertBefore){
                        type = InsertionType::BEFORE;
                    }else if (insertAfter){
                        type = InsertionType::AFTER;
                    }else{
                        type = InsertionType::INTO;
                    }
                    CommandHandle::get(project->getSelectedSceneId())->addCommand(new MoveEntityOrderCmd(project, project->getSelectedSceneId(), sourceEntity, node.id, type));
                }

            }

            if (!node.isScene && insertBefore) {
                const ImVec2& padding = ImGui::GetStyle().FramePadding;

                ImVec2 lineStart = ImGui::GetCursorScreenPos();
                ImVec2 lineEnd = lineStart;
                lineEnd.x += ImGui::GetContentRegionAvail().x;

                lineStart.y -= padding.y * 2.0;
                lineEnd.y -= padding.y * 2.0;

                ImVec2 node_size = ImGui::GetItemRectSize();

                lineStart.y -= node_size.y;
                lineEnd.y -= node_size.y;

                drawInsertionMarker(lineStart, lineEnd);
            }

            if (!node.isScene && insertAfter) {
                const ImVec2& padding = ImGui::GetStyle().FramePadding;

                ImVec2 lineStart = ImGui::GetCursorScreenPos();
                ImVec2 lineEnd = lineStart;
                lineEnd.x += ImGui::GetContentRegionAvail().x;

                lineStart.y -= padding.y;
                lineEnd.y -= padding.y;

                drawInsertionMarker(lineStart, lineEnd);
            }

        }

        if (ImGui::AcceptDragDropPayload("resource_files")) {
            // Parse the dropped file paths
            std::vector<std::string> droppedPaths;
            const char* data = (const char*)payload->Data;
            size_t dataSize = payload->DataSize;

            // Parse null-terminated strings from the payload
            size_t offset = 0;
            while (offset < dataSize) {
                std::string path(data + offset);
                if (!path.empty()) {
                    droppedPaths.push_back(path);
                }
                offset += path.size() + 1; // +1 for null terminator
            }

            // Dropping on the scene root: allow adding child scenes via .scene files,
            // and keep supporting .entity imports.
            if (node.isScene) {
                handleSceneFilesDropAsChildScenes(droppedPaths, node.id);
                handleEntityFilesDrop(droppedPaths, NULL_ENTITY);
            } else {
                // Determine the parent entity for the drop
                Entity parentEntity = NULL_ENTITY;
                // If dropping on an entity, use it as parent if it has transform
                if (node.hasTransform) {
                    parentEntity = node.id;
                }
                handleEntityFilesDrop(droppedPaths, parentEntity);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        project->clearSelectedEntities(project->getSelectedSceneId());
        selectedScenes.clear();
        project->setSelectedSceneForProperties(project->getSelectedSceneId());
    }

    // Check for selection on mouse release (not click) to allow drag without selection
    bool wasItemActivePrevFrame = ImGui::IsItemActive();
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (node.isChildScene) {
            project->clearSelectedEntities(project->getSelectedSceneId());
            selectedScenes.clear();
            selectedScenes.push_back(&node);
            project->setSelectedSceneForProperties(node.childSceneId);
        } else if (!node.isScene){
            ImGuiIO& io = ImGui::GetIO();
            if (!io.KeyShift){
                project->clearSelectedEntities(project->getSelectedSceneId());
            }
            project->addSelectedEntity(project->getSelectedSceneId(), node.id);
            project->setSelectedSceneForProperties(project->getSelectedSceneId());
        }else{
            project->clearSelectedEntities(project->getSelectedSceneId());
            selectedScenes.clear();
            selectedScenes.push_back(&node);
            project->setSelectedSceneForProperties(node.id);
        }
    }

    if (project->hasSelectedEntities(project->getSelectedSceneId())){
        selectedScenes.clear();
    }

    // Handle double-click on child scene to select it (kept for UX consistency)
    if (node.isChildScene && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        SceneProject* childScene = project->getScene(node.childSceneId);
        if (childScene) {
            project->openScene(childScene->filepath);
        }
    }

    std::string popupId = "##ContextMenu" + getNodeImGuiId(node);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        strncpy(nameBuffer, node.name.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::OpenPopup(popupId.c_str());
    }

    if (ImGui::BeginPopup(popupId.c_str())) {
        // Child scene context menu
        if (node.isChildScene) {
            ImGui::Text("Child Scene: %s", node.name.c_str());
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_ARROW_POINTER "  Select scene")) {
                project->setSelectedSceneId(node.childSceneId);
            }
            if (ImGui::MenuItem(ICON_FA_TRASH "  Remove child scene")) {
                CommandHandle::get(node.ownerSceneId)->addCommand(new RemoveChildSceneCmd(project, node.ownerSceneId, node.childSceneId));
            }
            ImGui::EndPopup();
        } else {
            // Regular entity/scene context menu
            ImGui::Text("Name:");

            ImGui::PushItemWidth(200);
            if (ImGui::InputText("##ChangeNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)){
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                if (nameBuffer[0] != '\0' && strcmp(nameBuffer, node.name.c_str()) != 0) {
                    if (node.isScene){
                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new SceneNameCmd(project, node.id, nameBuffer));
                    }else{
                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new EntityNameCmd(project, project->getSelectedSceneId(), node.id, nameBuffer));
                    }
                }
            }

            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_COPY"  Duplicate")){
                // Action for SubItem 1
            }
            if (ImGui::MenuItem(ICON_FA_TRASH"  Delete")){
                if (!node.isScene){
                    CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new DeleteEntityCmd(project, project->getSelectedSceneId(), node.id));
                }
            }
            if (node.isParentShared){
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_LOCK_OPEN"  Make this entity local", nullptr, false, node.isShared)){
                    if (node.isShared){
                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new MakeEntityLocalCmd(project, project->getSelectedSceneId(), node.id, node.parent));
                    }
                }
                if (ImGui::MenuItem(ICON_FA_LINK"  Make this entity shared", nullptr, false, !node.isShared)){
                    if (!node.isShared){
                        CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new MakeEntitySharedCmd(project, project->getSelectedSceneId(), node.id, node.parent));
                    }
                }
            }
            if (node.hasTransform || node.isScene){
                ImGui::Separator();
                static bool createSharedChild = false;
                if (node.isShared){
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y * 0.5f));
                    ImGui::Checkbox("Create new shared", &createSharedChild);
                    ImGui::PopStyleVar(1);
                }
                if (ImGui::BeginMenu(ICON_FA_CIRCLE_DOT"  Create child")){
                    showNewEntityMenu(node.isScene, node.id, createSharedChild);
                }
            }

            // Scene-specific options
            if (node.isScene) {
                ImGui::Separator();
                showAddChildSceneMenu();
            }

            if (!node.isScene) {
                SceneProject* sceneProject = project->getSelectedScene();
                if (sceneProject->scene->getSignature(node.id).test(sceneProject->scene->getComponentId<CameraComponent>())) {
                    ImGui::Separator();
                    if (node.isMainCamera) {
                        if (ImGui::MenuItem(ICON_FA_VIDEO"  Unset as Main Camera", nullptr, false, node.isMainCamera)) {
                            CommandHandle::get(project->getSelectedSceneId())->addCommand(new SetMainCameraCmd(project, project->getSelectedSceneId(), NULL_ENTITY));
                        }
                    } else {
                        if (ImGui::MenuItem(ICON_FA_VIDEO"  Set as Main Camera", nullptr, false, !node.isMainCamera)) {
                            CommandHandle::get(project->getSelectedSceneId())->addCommand(new SetMainCameraCmd(project, project->getSelectedSceneId(), node.id));
                        }
                    }
                }
            }

            ImGui::EndPopup();
        }
    }

    if (node.separator){
        ImGui::Separator();
    }

    if (nodeOpen) {
        for (auto& child : node.children) {
            showTreeNode(child);
        }
        ImGui::TreePop();
    }
}

std::string Editor::Structure::getNodeImGuiId(TreeNode& node){
    std::string id;
    if (node.isScene){
        id += "ItemScene";
        id += std::to_string(node.id);
    }else if (node.isChildScene){
        id += "ItemChildScene";
        id += std::to_string(node.ownerSceneId);
        id += "_";
        id += std::to_string(node.childSceneId);
    }else{
        id += "ItemScene";
        id += std::to_string(project->getSelectedSceneId());
        id += "Entity";
        id += std::to_string(node.id);
    }

    return id;
}

void Editor::Structure::show(){
    SceneProject* sceneProject = project->getSelectedScene();
    Entity mainCamera = sceneProject->mainCamera;
    size_t order = 0;

    TreeNode root;

    root.icon = ICON_FA_TV;
    root.id = sceneProject->id;
    root.isScene = true;
    root.order = order++;
    root.name = sceneProject->name;

    // child scenes (shown before entities)
    bool hasChildScenes = !sceneProject->childScenes.empty();
    for (const auto& childSceneId : sceneProject->childScenes) {
        const SceneProject* childScene = project->getScene(childSceneId);
        if (!childScene) {
            continue; // Skip invalid child scene references
        }

        TreeNode childSceneNode;
        childSceneNode.icon = ICON_FA_FILM;
        childSceneNode.name = childScene->name;
        childSceneNode.isChildScene = true;
        childSceneNode.childSceneId = childSceneId;
        childSceneNode.ownerSceneId = sceneProject->id;
        childSceneNode.order = order++;
        root.children.push_back(childSceneNode);
    }

    bool separatorAfterChildScenes = hasChildScenes;

    // non-hierarchical entities
    for (auto& entity : sceneProject->entities) {
        Signature signature = sceneProject->scene->getSignature(entity);

        if (!signature.test(sceneProject->scene->getComponentId<Transform>())){
            if (separatorAfterChildScenes && !root.children.empty()) {
                root.children.back().separator = true;
                separatorAfterChildScenes = false;
            }

            TreeNode child;
            child.icon = getObjectIcon(signature, sceneProject->scene);
            child.id = entity;
            child.isMainCamera = (entity == mainCamera);
            child.order = order++;
            child.name = sceneProject->scene->getEntityName(entity);

            root.children.push_back(child);
        }
    }

    bool applySeparator = false;
    if (root.children.size() > 0){
        applySeparator = true;
    }

    // hierarchical entities
    auto transforms = sceneProject->scene->getComponentArray<Transform>();
    for (int i = 0; i < transforms->size(); i++){
        Transform& transform = transforms->getComponentFromIndex(i);
        Entity entity = transforms->getEntity(i);
        Signature signature = sceneProject->scene->getSignature(entity);

        if (std::count(sceneProject->entities.begin(), sceneProject->entities.end(), entity) > 0){
            if (applySeparator){
                root.children.back().separator = true;
                applySeparator = false;
            }

            if (separatorAfterChildScenes && !root.children.empty()) {
                root.children.back().separator = true;
                separatorAfterChildScenes = false;
            }

            TreeNode child;
            child.icon = getObjectIcon(signature, sceneProject->scene);
            child.id = entity;
            child.isMainCamera = (entity == mainCamera);
            child.hasTransform = true;
            child.order = order++;
            child.name = sceneProject->scene->getEntityName(entity);
            if (transform.parent == NULL_ENTITY){
                root.children.push_back(child);
            }else{
                TreeNode* parent = findNode(&root, transform.parent);
                if (parent){
                    child.parent = parent->id;
                    parent->children.push_back(child);
                }else{
                    printf("ERROR: Could not find parent of entity %u\n", entity);
                }
            }
        }
    }

    ImGui::Begin("Structure");

    showIconMenu();
    ImGui::BeginChild("StructureScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    // Apply search filtering if there's a search term
    if (strlen(searchBuffer) > 0) {
        std::string searchStr = searchBuffer;
        if (!matchCase) {
            std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);
        }
        markMatchingNodes(root, searchStr);
    }

    showTreeNode(root);

    // Handle right-click in empty space
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
        ImGui::OpenPopup("EmptySpaceContextMenu");
    }

    // Empty space context menu
    if (ImGui::BeginPopup("EmptySpaceContextMenu")) {
        if (ImGui::BeginMenu(ICON_FA_CIRCLE_DOT"  Create entity")) {
            showNewEntityMenu(true, NULL_ENTITY, false);
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();

    // Handle drag and drop for entity files
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource_files")) {
            // Parse the dropped file paths
            std::vector<std::string> droppedPaths;
            const char* data = (const char*)payload->Data;
            size_t dataSize = payload->DataSize;

            // Parse null-terminated strings from the payload
            size_t offset = 0;
            while (offset < dataSize) {
                std::string path(data + offset);
                if (!path.empty()) {
                    droppedPaths.push_back(path);
                }
                offset += path.size() + 1; // +1 for null terminator
            }

            handleSceneFilesDropAsChildScenes(droppedPaths, project->getSelectedSceneId());
            handleEntityFilesDrop(droppedPaths);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}