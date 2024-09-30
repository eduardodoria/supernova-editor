#include "Objects.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Objects::Objects(){
}

void Editor::Objects::showIconMenu(){
    static char inputText[256] = "";

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
    ImGui::InputText("##hiddenLabel", inputText, IM_ARRAYSIZE(inputText));

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
        if (ImGui::MenuItem("Scene"))
        {
            // Action for Item 1
        }

        if (ImGui::BeginMenu("Basic shape"))
        {
            if (ImGui::MenuItem("Box"))
            {
                // Action for SubItem 1
            }
            if (ImGui::MenuItem("Plane"))
            {
                // Action for SubItem 2
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Model"))
        {
            // Action for Item 2
        }

        ImGui::EndPopup();
    }
}

void Editor::Objects::showTreeNode(Editor::TreeNode& node) {
    static char buffer[256];
    static TreeNode* selectedNode = nullptr;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (node.isScene){
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    }

    bool nodeOpen = ImGui::TreeNodeEx((node.icon + "  " + node.name).c_str(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("TreeNodeContextMenu");
        selectedNode = &node;
        strncpy(buffer, node.name.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
    }

    if (ImGui::BeginPopup("TreeNodeContextMenu")) {
        if (selectedNode == &node) {
            //ImGui::AlignTextToFramePadding();
            ImGui::Text("Name:");
            //ImGui::SameLine();

            ImGui::PushItemWidth(200);
            if (ImGui::InputText("##ChangeNameInput", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                selectedNode->name = buffer;
                ImGui::CloseCurrentPopup();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate"))
            {
                // Action for SubItem 1
            }
            if (ImGui::BeginMenu("Create child"))
            {
                if (ImGui::BeginMenu("Basic shape"))
                {
                    if (ImGui::MenuItem("Box"))
                    {
                        // Action for SubItem 1
                    }
                    if (ImGui::MenuItem("Plane"))
                    {
                        // Action for SubItem 2
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Model"))
                {
                    // Action for Item 2
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndPopup();
    }else if (selectedNode == &node) {
        // Update the name when popup is closed
        selectedNode->name = buffer;
        selectedNode = nullptr;
    }

    if (nodeOpen) {
        for (auto& child : node.children) {
            showTreeNode(child);
        }
        ImGui::TreePop();
    }
}

void Editor::Objects::show(Project* project){

    static TreeNode root = {ICON_FA_TV, "Root Node", true, {
        {ICON_FA_CUBE, "Player", false, {}},
        {ICON_FA_CUBE, "Child 2", false, {
            {ICON_FA_CUBE, "Grandchild 1", false, {}},
            {ICON_FA_CUBE, "Grandchild 2", false, {}}
        }},
        {ICON_FA_FILE, "Child 3", false, {}}
    }};

    ImGui::Begin("Objects");
    showIconMenu();
    showTreeNode(root);
    ImGui::End();
}