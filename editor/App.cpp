#include "App.h"

#include "imgui_internal.h"
#include "Platform.h"
#include "Supernova.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

bool Editor::App::isInitialized = false;
unsigned int Editor::App::texture;

Editor::App::App(){

}

void Editor::App::showMenu(){
    // Remove menu bar border
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Create the main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
                // Handle open action
            }
            if (ImGui::MenuItem("Save")) {
                // Handle save action
            }
            if (ImGui::MenuItem("Exit")) {
                // Handle exit action
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                // Handle undo action
            }
            if (ImGui::MenuItem("Redo")) {

                // Handle redo action
            }
            if (ImGui::MenuItem("Reset layout")) {
                buildDockspace();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Restore previous style
    ImGui::PopStyleVar(2);
}

void Editor::App::buildDockspace(){
    ImGuiID dock_id_left, dock_id_right, dock_id_middle, dock_id_middle_top, dock_id_middle_bottom;

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Split the dockspace into left and middle
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.0f, &dock_id_left, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_left, ImVec2(200, ImGui::GetMainViewport()->Size.y)); // Set left node size
    ImGui::DockBuilderDockWindow("Objects", dock_id_left);

    // Split the middle into right and remaining middle
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Right, 0.0f, &dock_id_right, &dock_id_middle);
    ImGui::DockBuilderSetNodeSize(dock_id_right, ImVec2(300, ImGui::GetMainViewport()->Size.y)); // Set right node size
    ImGui::DockBuilderDockWindow("Properties", dock_id_right);

    // Split the remaining middle into top and bottom
    ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down, 0.0f, &dock_id_middle_bottom, &dock_id_middle_top);
    ImGui::DockBuilderSetNodeSize(dock_id_middle_bottom, ImVec2(ImGui::GetMainViewport()->Size.x, 150)); // Set bottom node size
    ImGui::DockBuilderDockWindow("View", dock_id_middle_top);
    ImGui::DockBuilderDockWindow("Console", dock_id_middle_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}

void Editor::App::show(){
    dockspace_id = ImGui::GetID("MyDockspace");

    showMenu();

    if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
        buildDockspace();
    }

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::Begin("View");

    // Add a callback to execute custom rendering code
    //ImGui::GetWindowDrawList()->AddCallback(engineRender, nullptr);
    //ImGui::GetWindowDrawList()->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    ImGui::BeginChild("GameRender");
    {
        float width = ImGui::GetContentRegionAvail().x;
        float height = ImGui::GetContentRegionAvail().y;

        if (Platform::width != width || Platform::height != height){
            Platform::width = width;
            Platform::height = height;
            Engine::systemViewChanged();
        }

        ImGui::Image((void*)(intptr_t)texture, ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::EndChild();


    ImGui::End();
    
    objectsWindow.show();
    consoleWindow.show();
    propertiesWindow.show();
}

void Editor::App::engineRender(const ImDrawList* parent_list, const ImDrawCmd* cmd){
    const int cx = (int) cmd->ClipRect.x;
    const int cy = (int) cmd->ClipRect.y;
    const int cw = (int) (cmd->ClipRect.z - cmd->ClipRect.x);
    const int ch = (int) (cmd->ClipRect.w - cmd->ClipRect.y);

    Platform::width = cw;
    Platform::height = cw;

    if (!isInitialized){
        //Engine::systemViewLoaded();
        Engine::systemViewChanged();
        isInitialized = true;
    }

    Engine::systemDraw();

    printf("%i %i %i %i\n", cx,cy,cw,ch);
}