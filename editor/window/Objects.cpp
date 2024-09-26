#include "Objects.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Objects::Objects(){
}

void Editor::Objects::showIconMenu(){
    static char inputText[256] = "";


    ImGui::Begin("Objects");

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

void Editor::Objects::show(){

    showIconMenu();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(ICON_FA_ADDRESS_BOOK" Root Node"))
    {
        if (ImGui::TreeNode(ICON_FA_CUBE" Player"))
        {
            ImGui::Text("Position: (0, 0, 0)");
            ImGui::Text("Health: 100");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode(ICON_FA_CUBE" Environment"))
        {
            if (ImGui::TreeNode(ICON_FA_CUBE" Tree"))
            {
                ImGui::Text("Type: Oak");
                ImGui::Text("Height: 10m");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode(ICON_FA_CUBE" Rock"))
            {
                ImGui::Text("Size: Large");
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode(ICON_FA_CUBE" Enemies"))
        {
            if (ImGui::TreeNode(ICON_FA_CUBE" Goblin"))
            {
                ImGui::Text("Health: 30");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode(ICON_FA_CUBE" Dragon"))
            {
                ImGui::Text("Health: 300");
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
    ImGui::End();
}