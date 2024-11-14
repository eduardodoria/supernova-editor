#include "Structure.h"

#include "external/IconsFontAwesome6.h"
#include "command/type/MoveEntityOrderCmd.h"

using namespace Supernova;

Editor::Structure::Structure(Project* project){
    this->project = project;
    this->contextMenuOpened = "";
    this->openParent = NULL_ENTITY;
}

void Editor::Structure::showNewEntityMenu(bool isScene, Entity parent){
    if (isScene){
        parent = NULL_ENTITY;

        if (ImGui::MenuItem(ICON_FA_CIRCLE_DOT"  Empty entity"))
        {
            project->createEmptyEntity(project->getSelectedSceneId());
        }
    }

    if (ImGui::BeginMenu(ICON_FA_CUBE"  Basic shape"))
    {
        if (ImGui::MenuItem(ICON_FA_CUBE"  Box"))
        {
            project->createBoxShape(project->getSelectedSceneId(), parent);
            openParent = parent;
        }
        if (ImGui::MenuItem(ICON_FA_CUBE"  Plane"))
        {
            // Action for SubItem 2
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem(ICON_FA_PERSON_RUNNING"  Model"))
    {
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
    ImGui::InputText("##hiddenLabel", searchBuffer, IM_ARRAYSIZE(searchBuffer));

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
            project->createNewScene("New Scene");
        }
        //ImGui::SameLine();
        if (ImGui::Button(ICON_FA_CUBES_STACKED "  2D Scene", buttonSize)) {
            // Handle play button click
        }
        //ImGui::SameLine();
        if (ImGui::Button(ICON_FA_WINDOW_RESTORE "  UI Scene", buttonSize)) {
            // Handle play button click
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

void Editor::Structure::showTreeNode(Editor::TreeNode& node) {
    static std::vector<TreeNode*> selectedNodes;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (std::find(selectedNodes.begin(), selectedNodes.end(), &node) != selectedNodes.end()) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    if(!node.isScene && openParent == node.id) {
        ImGui::SetNextItemOpen(true);
        openParent = NULL_ENTITY;
    }

    bool nodeOpen = ImGui::TreeNodeEx((node.icon + "  " + node.name + "###" + getNodeImGuiId(node)).c_str(), flags);

    std::string dragDropName = "ENTITY";
    if (node.hasTransform){
        dragDropName = dragDropName + "_T";
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload(dragDropName.c_str(), &node, sizeof(TreeNode));
        ImGui::Text("Moving %s", node.name.c_str());
        ImGui::EndDragDropSource();
    }

    bool insertBefore = false;
    bool insertAfter = false;

    if (ImGui::BeginDragDropTarget()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemMax = ImGui::GetItemRectMax();
        if (!node.isScene && project->getSelectedScene()->scene->findComponent<Transform>(node.id)){
            insertBefore = (mousePos.y - itemMin.y) < (itemMax.y - itemMin.y) * 0.2f;
            insertAfter = (mousePos.y - itemMin.y) > (itemMax.y - itemMin.y) * 0.8f;
        }else{
            insertBefore = (mousePos.y - itemMin.y) < (itemMax.y - itemMin.y) * 0.5f;
            insertAfter = (mousePos.y - itemMin.y) >= (itemMax.y - itemMin.y) * 0.5f;
        }

        //ImGuiDragDropFlags flags = 0;
        ImGuiDragDropFlags flags = ImGuiDragDropFlags_AcceptBeforeDelivery;

        if (insertBefore || insertAfter){
            flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragDropName.c_str(), flags)) {
            TreeNode* source = (TreeNode*)payload->Data;

            if (node.parent == source->parent){
                if (node.order == (source->order+1)){
                    insertBefore = false;
                }
                if (node.order == (source->order-1)){
                    insertAfter = false;
                }
            }

            if (!source->isScene && !node.isScene && payload->IsDelivery()){
                //printf("Dropped: %s in %s\n", source->name.c_str(), node.name.c_str());
                InsertionType type;
                if (insertBefore){
                    type = InsertionType::BEFORE;
                }else if (insertAfter){
                    type = InsertionType::AFTER;
                }else{
                    type = InsertionType::IN;
                }
                CommandHistory::addCommand(new MoveEntityOrderCmd(project, project->getSelectedSceneId(), source->id, node.id, type));
            }

        }
        ImGui::EndDragDropTarget();
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

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        //TODO
        project->setSelectedEntity(project->getSelectedSceneId(), NULL_ENTITY);
    }

    if (!project->hasSelectedEntities(project->getSelectedSceneId())){
        selectedNodes.clear();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        selectedNodes.clear();
        selectedNodes.push_back(&node);
        if (!node.isScene){
            project->setSelectedEntity(project->getSelectedSceneId(), node.id);
        }else{
            //TODO
            project->setSelectedEntity(project->getSelectedSceneId(), NULL_ENTITY);
        }
    }
    if (!node.isScene && project->isSelectedEntity(project->getSelectedSceneId(), node.id)){
        selectedNodes.push_back(&node);
    }

    std::string popupId = "##ContextMenu" + getNodeImGuiId(node);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        strncpy(nameBuffer, node.name.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::OpenPopup(popupId.c_str());
    }

    if (ImGui::BeginPopup(popupId.c_str())) {
        contextMenuOpened = popupId;

        ImGui::Text("Name:");

        ImGui::PushItemWidth(200);
        bool enterPressed = ImGui::InputText("##ChangeNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
        if (enterPressed) {
            if (nameBuffer[0] != '\0') {
                changeNodeName(&node, nameBuffer);
                ImGui::CloseCurrentPopup();
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

    if (!ImGui::IsPopupOpen(popupId.c_str()) && contextMenuOpened == popupId){
        contextMenuOpened = "";
        if (nameBuffer[0] != '\0' && strcmp(nameBuffer, node.name.c_str()) != 0) {
            changeNodeName(&node, nameBuffer);
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
    }else{
        id += "ItemScene";
        id += std::to_string(project->getSelectedSceneId());
        id += "Entity";
    }
    id += std::to_string(node.id);

    return id;
}

void Editor::Structure::changeNodeName(const TreeNode* node, const std::string name){
    if (node->isScene){
        project->getScene(node->id)->name = name;
    }else{
        project->getSelectedScene()->scene->setEntityName(node->id, name);
    }
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
    showTreeNode(root);
    ImGui::End();
}