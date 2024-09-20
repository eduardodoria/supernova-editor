#include "Objects.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Objects::Objects(){
}

void Editor::Objects::show(){
    ImGui::Begin("Objects");
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