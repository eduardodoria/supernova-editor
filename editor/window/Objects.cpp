#include "Objects.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Objects::Objects(Project* project){
    this->project = project;

    this->selectedNodeRight = nullptr;
}

void Editor::Objects::showNewEntityMenu(){
    if (ImGui::BeginMenu(ICON_FA_CUBE"  Basic shape"))
    {
        if (ImGui::MenuItem(ICON_FA_CUBE"  Box"))
        {
            project->createBoxShape(project->getSelectedSceneId());
            //printf("%u\n", selectedNode->id);
            // Action for SubItem 1
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

    if (ImGui::MenuItem(ICON_FA_CIRCLE_DOT"  Empty entity"))
    {
        project->createEmptyEntity(project->getSelectedSceneId());
        // Action for Item 2
    }
    ImGui::EndMenu();
}

void Editor::Objects::showIconMenu(){
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
            // Handle play button click
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
            showNewEntityMenu();
        }

        ImGui::EndPopup();
    }
}

void Editor::Objects::showTreeNode(Editor::TreeNode& node) {
    static TreeNode* selectedNode = nullptr;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (selectedNode == &node) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (node.isScene){
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    bool nodeOpen = ImGui::TreeNodeEx((node.icon + "  " + node.name).c_str(), flags);

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("TREE_NODE", &node, sizeof(TreeNode));
        ImGui::Text("Moving %s", node.name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TREE_NODE")) {
            TreeNode* source = (TreeNode*)payload->Data;
            //node.children.push_back(*payload_node);
            printf("Dropped: %s in %s\n", source->name.c_str(), node.name.c_str());
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        selectedNode = nullptr;
        project->setSelectedEntity(project->getSelectedSceneId(), NULL_ENTITY);
    }

    if (project->getSelectedEntity(project->getSelectedSceneId()) == NULL_ENTITY){
        selectedNode = nullptr;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        selectedNode = &node;
        if (!node.isScene){
            project->setSelectedEntity(project->getSelectedSceneId(), node.id);
        }else{
            project->setSelectedEntity(project->getSelectedSceneId(), NULL_ENTITY);
        }
    }
    if (!node.isScene && project->getSelectedEntity(project->getSelectedSceneId()) == node.id){
        selectedNode = &node;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("TreeNodeContextMenu");
        selectedNodeRight = &node;
        strncpy(nameBuffer, node.name.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    }

    if (ImGui::BeginPopup("TreeNodeContextMenu")) {
        if (selectedNodeRight == &node) {
            //ImGui::AlignTextToFramePadding();
            ImGui::Text("Name:");
            //ImGui::SameLine();

            ImGui::PushItemWidth(200);
            if (ImGui::InputText("##ChangeNameInput", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (nameBuffer[0] != '\0'){
                    changeNodeName(selectedNodeRight, nameBuffer);

                    selectedNodeRight = nullptr;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_COPY"  Duplicate"))
            {
                // Action for SubItem 1
            }
            if (ImGui::MenuItem(ICON_FA_TRASH"  Delete"))
            {
                if (!selectedNodeRight->isScene){
                    project->deleteEntity(project->getSelectedSceneId(), selectedNodeRight->id);
                }
            }
            ImGui::Separator();
            if (ImGui::BeginMenu(ICON_FA_CIRCLE_DOT"  Create child"))
            {
                showNewEntityMenu();
            }
        }
        ImGui::EndPopup();
    }else if (selectedNodeRight == &node) {
        // Update the name when popup is closed
        if (nameBuffer[0] != '\0'){
            changeNodeName(selectedNodeRight, nameBuffer);
        }
        selectedNodeRight = nullptr;
    }

    if (nodeOpen) {
        for (auto& child : node.children) {
            showTreeNode(child);
        }
        ImGui::TreePop();
    }
}

void Editor::Objects::changeNodeName(const TreeNode* node, const std::string name){
    if (node->isScene){
        project->getScene(node->id)->name = name;
    }else{
        project->getSelectedScene()->scene->setEntityName(node->id, name);
    }
}

void Editor::Objects::show(){
/*
    static TreeNode root = {ICON_FA_TV, "Root Node", true, 1, {
        {ICON_FA_CUBE, "Player", false, 1, {}},
        {ICON_FA_CUBE, "Child 2", false, 1, {
            {ICON_FA_CUBE, "Grandchild 1", false, 1, {}},
            {ICON_FA_CUBE, "Grandchild 2", false, 1, {}}
        }},
        {ICON_FA_CIRCLE_DOT, "Entity 3", false, 1, {}}
    }};
*/

    SceneData* sceneData = project->getSelectedScene();

    TreeNode root;

    root.icon = ICON_FA_TV;
    root.id = sceneData->id;
    root.isScene = true;
    root.name = sceneData->name;

    for (auto& entity : project->getSelectedScene()->entities) {
        TreeNode child;

        child.icon = ICON_FA_CIRCLE_DOT;
        child.id = entity;
        child.isScene = false;
        child.name = project->getSelectedScene()->scene->getEntityName(entity);

        root.children.push_back(child);
    }

    ImGui::Begin("Objects");
    showIconMenu();
    showTreeNode(root);
    ImGui::End();
}