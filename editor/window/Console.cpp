#include "Console.h"
#include "external/IconsFontAwesome6.h"
#include <ctime>
#include <iomanip>
#include <sstream>

#include "imgui_internal.h"

using namespace Supernova::Editor;

Console::Console() {
    autoScroll = true;
    autoScrollLocked = true;
    scrollStartCount = 0;
    clear();
}

void Console::clear() {
    buf.clear();
    logs.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);
}

void Console::addLog(LogType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char temp[4096];
    vsnprintf(temp, IM_ARRAYSIZE(temp), fmt, args);
    va_end(args);

    addLog(type, std::string(temp));
}

void Console::addLog(LogType type, const std::string& message) {
    // Get current time
    //auto now = std::time(nullptr);
    //auto tm = *std::localtime(&now);
    //std::ostringstream timeStr;
    //timeStr << std::put_time(&tm, "%H:%M:%S");

    // Create the log entry
    LogData logEntry{type, message, static_cast<float>(ImGui::GetTime())};
    logs.push_back(logEntry);

    // Format the message with timestamp and type
    std::string typeStr;
    switch(type) {
        case LogType::Error: typeStr = "Error"; break;
        case LogType::Success: typeStr = "Success"; break;
        case LogType::Info: typeStr = "Info"; break;
        case LogType::Warning: typeStr = "Warning"; break;
        case LogType::Build: typeStr = "Build"; break;
    }

    std::string formattedMessage = "[" + typeStr + "] " + message + "\n";

    // Add to ImGui buffer
    int oldSize = buf.size();
    buf.append(formattedMessage.c_str());
    for (int newSize = buf.size(); oldSize < newSize; oldSize++)
        if (buf[oldSize] == '\n')
            lineOffsets.push_back(oldSize + 1);
}

void Console::rebuildBuffer() {
    buf.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);

    for (const auto& log : logs) {
        std::string typeStr;
        switch(log.type) {
            case LogType::Error: typeStr = "Error"; break;
            case LogType::Success: typeStr = "Success"; break;
            case LogType::Info: typeStr = "Info"; break;
            case LogType::Warning: typeStr = "Warning"; break;
            case LogType::Build: typeStr = "Build"; break;
        }

        std::string formattedMessage = "[" + typeStr + "] " + log.message + "\n";

        if (!filter.IsActive() || filter.PassFilter(formattedMessage.c_str())) {
            int oldSize = buf.size();
            buf.append(formattedMessage.c_str());
            for (int newSize = buf.size(); oldSize < newSize; oldSize++)
                if (buf[oldSize] == '\n')
                    lineOffsets.push_back(oldSize + 1);
        }
    }
}

void Console::show() {
    if (!ImGui::Begin("Console")) {
        ImGui::End();
        return;
    }

    // Calculate dimensions for the vertical menu and main content
    const float menuWidth = ImGui::CalcTextSize(ICON_FA_LOCK).x + ImGui::GetStyle().ItemSpacing.x * 2 + ImGui::GetStyle().FramePadding.x * 2;  // Width of the vertical menu
    //const float spacing = 8.0f;     // Spacing between menu and content
    const ImVec2 windowSize = ImGui::GetContentRegionAvail();

    // Begin vertical menu
    ImGui::BeginChild("VerticalMenu", ImVec2(menuWidth, windowSize.y), true);

    // Lock button (lock/unlock icon) controlling autoScroll
    if (ImGui::Button(autoScrollLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN, ImVec2(ImGui::CalcTextSize(ICON_FA_LOCK).x + ImGui::GetStyle().FramePadding.x * 2, 0.0f))) {
        autoScrollLocked = !autoScrollLocked;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(autoScrollLocked ? "Disable Auto-scroll" : "Enable Auto-scroll");
    }

    // Filter button (filter icon)
    bool filterActive = filter.IsActive();
    if (filterActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.5f, 1.0f));
    }
    if (ImGui::Button(ICON_FA_FILTER)) {
        ImGui::OpenPopup("FilterPopup");
    }
    if (filterActive) {
        ImGui::PopStyleColor();
    }
    if (ImGui::BeginPopup("FilterPopup")) {
        if (filter.Draw("##filter", 200.0f)) {
            rebuildBuffer();
        }
        ImGui::EndPopup();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Filter Logs");
    }

    // Clear button (trash icon)
    if (ImGui::Button(ICON_FA_TRASH)) {
        clear();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Clear Console");
    }

    ImGui::EndChild();

    // Main content area
    ImGui::SameLine(menuWidth);

    // Create a read-only InputTextMultiline
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::InputTextMultiline("##console", (char*)buf.begin(), buf.size(), 
        ImVec2(-FLT_MIN, -FLT_MIN), 
        ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGuiContext& g = *GImGui;
    const char* child_window_name = NULL;
    ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X", g.CurrentWindow->Name, "##console", ImGui::GetID("##console"));
    ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);

    if (child_window->ScrollMax.y > 0.0f){
        if (child_window->Scroll.y < (child_window->ScrollMax.y-ImGui::GetTextLineHeight())) {
            autoScroll = false;
        }
        // Re-enable auto-scroll if user scrolls to bottom manually
        if (child_window->Scroll.y >= child_window->ScrollMax.y) {
            autoScroll = true;
        }
    }

    if (autoScrollLocked){
        if (autoScroll) {
            ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
        }else if (scrollStartCount < 3){ // keep scroll at window start
            scrollStartCount++;
            ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
        }
    }

    ImGui::End();
}