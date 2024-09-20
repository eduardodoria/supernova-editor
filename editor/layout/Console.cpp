#include "Console.h"

#include "imgui.h"

#include "external/IconsFontAwesome6.h"

using namespace Supernova;

Editor::Console::Console(){
}

void Editor::Console::show(){
    ImGui::Begin("Console");
    // Example of colored text output
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: Something went wrong!");
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Success: Operation completed.");
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Info: System is running smoothly.");
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning: Check your settings.");
    ImGui::End();
}