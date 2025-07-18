#include "Structure.h"

#include "external/IconsFontAwesome6.h"
#include "command/CommandHandle.h"
#include "command/type/MoveEntityOrderCmd.h"
#include "command/type/CreateEntityCmd.h"
#include "command/type/EntityNameCmd.h"
#include "command/type/SceneNameCmd.h"
#include "util/EntityPayload.h"
#include "Out.h"
#include "Stream.h"

using namespace Supernova;

Editor::Structure::Structure(Project* project, SceneWindow* sceneWindow){
    this->project = project;
    this->sceneWindow = sceneWindow;
    this->openParent = NULL_ENTITY;
}

void Editor::Structure::showNewEntityMenu(bool isScene, Entity parent){
    if (isScene){
        parent = NULL_ENTITY;

        if (ImGui::MenuItem(ICON_FA_CIRCLE_DOT"  Empty entity")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Entity"));
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem(ICON_FA_SITEMAP"  Empty object")){
        CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Object", EntityCreationType::OBJECT, parent));
        openParent = parent;
    }

    if (ImGui::BeginMenu(ICON_FA_CUBE"  Basic shape")){
        if (ImGui::MenuItem(ICON_FA_CUBE"  Box")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Box", EntityCreationType::BOX, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Plane")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Plane", EntityCreationType::PLANE, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Sphere")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Sphere", EntityCreationType::SPHERE, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Cylinder")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Cylinder", EntityCreationType::CYLINDER, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Capsule")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Capsule", EntityCreationType::CAPSULE, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Torus")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Torus", EntityCreationType::TORUS, parent));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_CUBES_STACKED"  2D")){
        if (ImGui::MenuItem(ICON_FA_IMAGE"  Sprite")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Sprite", EntityCreationType::SPRITE, parent));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_WINDOW_RESTORE"  UI")){
        if (ImGui::MenuItem(ICON_FA_IMAGE"  Image")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Image", EntityCreationType::IMAGE, parent));
            openParent = parent;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_LIGHTBULB"  Light")){
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Point")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Point Light", EntityCreationType::POINT_LIGHT, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Directional")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Directional Light", EntityCreationType::DIRECTIONAL_LIGHT, parent));
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_LIGHTBULB"  Spot")){
            CommandHandle::get(project->getSelectedSceneId())->addCommand(new CreateEntityCmd(project, project->getSelectedSceneId(), "Spot Light", EntityCreationType::SPOT_LIGHT, parent));
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

    // Get default sizes
    float inputHeight = ImGui::GetFrameHeight();
    ImVec2 buttonSize = ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS);
    buttonSize.x += ImGui::GetStyle().FramePadding.x * 2.0f;
    buttonSize.y = inputHeight;

    // Create a group for the input text with button
    ImGui::BeginGroup();

    // Input text
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonSize.x);
    ImGui::InputText("##structure_search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

    // Button inside input with same color as input background
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
    if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS))
    {
        // Button logic here
    }
    ImGui::PopStyleColor(3);

    ImGui::PopItemWidth();
    ImGui::EndGroup();

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
            showNewEntityMenu(true, NULL_ENTITY);
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
    }else if (signature.test(scene->getComponentId<UIComponent>())){
        return ICON_FA_IMAGE;
    }else if (signature.test(scene->getComponentId<LightComponent>())){
        return ICON_FA_LIGHTBULB;
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

void Editor::Structure::handleEntityFilesDrop(const std::vector<std::string>& filePaths) {
    for (const std::string& filePath : filePaths) {
        std::filesystem::path path(filePath);

        // Check if it's an entity file
        if (path.extension() == ".entity") {
            // Import the shared entity into the current scene
            bool success = project->importSharedEntity(project->getSelectedSceneId(), path);

            if (success) {
                Out::info("Successfully imported entity from: %s", path.string().c_str());
            } else {
                Out::warning("Failed to import entity from: %s (might already exist in scene)", path.string().c_str());
            }
        }
    }
}

void Editor::Structure::showTreeNode(Editor::TreeNode& node) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (std::find(selectedScenes.begin(), selectedScenes.end(), &node) != selectedScenes.end()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (project->isSelectedEntity(project->getSelectedSceneId(), node.id)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    if(!node.isScene && openParent == node.id) {
        ImGui::SetNextItemOpen(true);
        openParent = NULL_ENTITY;
    }

    // Check if entity is shared and get shared group info
    bool isShared = false;
    std::filesystem::path sharedFilepath;
    const Editor::SharedGroup* sharedGroup = nullptr;

    if (!node.isScene) {
        sharedFilepath = project->findGroupFor(project->getSelectedSceneId(), node.id);
        isShared = !sharedFilepath.empty();
        if (isShared) {
            sharedGroup = project->getSharedGroup(sharedFilepath);
        }
    }

    // Push blue color for shared entities
    if (isShared) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f)); // Light blue color
    }

    bool nodeOpen = ImGui::TreeNodeEx((node.icon + "  " + node.name + "###" + getNodeImGuiId(node)).c_str(), flags);

    // Pop color if we pushed it
    if (isShared) {
        ImGui::PopStyleColor();
    }

    if (node.isScene){
        ImGui::SetItemTooltip("Id: %u", node.id);
    }else{
        if (isShared && sharedGroup) {
            std::filesystem::path relativePath = std::filesystem::relative(sharedFilepath, project->getProjectPath());
            ImGui::SetItemTooltip("Entity: %u (Shared)\nPath: %s", node.id, relativePath.string().c_str());
        } else {
            ImGui::SetItemTooltip("Entity: %u", node.id);
        }
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        // Add entity drag drop payload for dragging to resources
        if (!node.isScene) {
            Scene* scene = project->getSelectedScene()->scene;
            YAML::Node entityData = Stream::encodeEntity(node.id, scene);
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

    if (ImGui::BeginDragDropTarget()) {
        bool allowDragDrop = true;
        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
        Entity sourceEntity = 0;
        Entity sourceParent = 0;
        size_t sourceOrder = 0;
        bool sourceHasTransform = false;
        if (payload && payload->IsDataType("entity")) {
            if (payload->DataSize >= sizeof(EntityPayload)) {
                const EntityPayload* p = reinterpret_cast<const EntityPayload*>(payload->Data);
                sourceEntity = p->entity;
                sourceParent = p->parent;
                sourceOrder = p->order;
                sourceHasTransform = p->hasTransform;
            }

            if (sourceHasTransform != node.hasTransform) {
                allowDragDrop = false;
            }
        }

        if (allowDragDrop){

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

            ImGuiDragDropFlags flags = 0;
            //ImGuiDragDropFlags flags = ImGuiDragDropFlags_AcceptBeforeDelivery;

            if (insertBefore || insertAfter){
                flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
            }

            if (ImGui::AcceptDragDropPayload("entity", flags)) {
                if (node.parent == sourceParent){
                    if (node.order == (sourceOrder+1)){
                        insertBefore = false;
                    }
                    if (node.order == (sourceOrder-1)){
                        insertAfter = false;
                    }
                }

                if (!node.isScene && payload->IsDelivery()){
                    InsertionType type;
                    if (insertBefore){
                        type = InsertionType::BEFORE;
                    }else if (insertAfter){
                        type = InsertionType::AFTER;
                    }else{
                        type = InsertionType::IN;
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

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        project->clearSelectedEntities(project->getSelectedSceneId());
        selectedScenes.clear();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        if (!node.isScene){
            ImGuiIO& io = ImGui::GetIO();
            if (!io.KeyShift){
                project->clearSelectedEntities(project->getSelectedSceneId());
            }
            project->addSelectedEntity(project->getSelectedSceneId(), node.id);
        }else{
            project->clearSelectedEntities(project->getSelectedSceneId());
            selectedScenes.clear();
            selectedScenes.push_back(&node);
        }
    }

    if (project->hasSelectedEntities(project->getSelectedSceneId())){
        selectedScenes.clear();
    }

    std::string popupId = "##ContextMenu" + getNodeImGuiId(node);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        strncpy(nameBuffer, node.name.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::OpenPopup(popupId.c_str());
    }

    if (ImGui::BeginPopup(popupId.c_str())) {
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
                    CommandHandle::get(project->getSelectedSceneId())->addCommandNoMerge(new EntityNameCmd(project->getSelectedScene(), node.id, nameBuffer));
                }
            }
        }

        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_COPY"  Duplicate")){
            // Action for SubItem 1
        }
        if (ImGui::MenuItem(ICON_FA_TRASH"  Delete")){
            if (!node.isScene){
                project->deleteEntity(project->getSelectedSceneId(), node.id);
            }
        }
        if (node.hasTransform || node.isScene){
            ImGui::Separator();
            if (ImGui::BeginMenu(ICON_FA_CIRCLE_DOT"  Create child")){
                showNewEntityMenu(node.isScene, node.id);
            }
        }
        ImGui::EndPopup();
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
    }else{
        id += "ItemScene";
        id += std::to_string(project->getSelectedSceneId());
        id += "Entity";
    }
    id += std::to_string(node.id);

    return id;
}

void Editor::Structure::show(){
    SceneProject* sceneProject = project->getSelectedScene();
    size_t order = 0;

    TreeNode root;

    root.icon = ICON_FA_TV;
    root.id = sceneProject->id;
    root.isScene = true;
    root.separator = false;
    root.hasTransform = false;
    root.order = order++;
    root.parent = 0;
    root.name = sceneProject->name;

    // non-hierarchical entities
    for (auto& entity : sceneProject->entities) {
        Signature signature = sceneProject->scene->getSignature(entity);

        if (!signature.test(sceneProject->scene->getComponentId<Transform>())){
            TreeNode child;
            child.icon = getObjectIcon(signature, sceneProject->scene);
            child.id = entity;
            child.isScene = false;
            child.separator = false;
            child.hasTransform = false;
            child.order = order++;
            child.parent = 0;
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

            TreeNode child;
            child.icon = getObjectIcon(signature, sceneProject->scene);
            child.id = entity;
            child.isScene = false;
            child.separator = false;
            child.hasTransform = true;
            child.order = order++;
            child.parent = 0;
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
    //if (ImGui::BeginPopupContextWindow("EmptyAreaContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
    //    ImGui::Text("TODO");
    //    ImGui::EndPopup();
    //}
    showIconMenu();
    ImGui::BeginChild("StructureScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    showTreeNode(root);
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

            handleEntityFilesDrop(droppedPaths);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}