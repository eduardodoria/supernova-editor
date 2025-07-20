#include "OutputWindow.h"
#include "external/IconsFontAwesome6.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "imgui_internal.h"

using namespace Supernova::Editor;

OutputWindow::OutputWindow() {
    autoScroll = true;
    autoScrollLocked = true;
    scrollStartCount = 0;
    needsRebuild = false;
    menuWidth = 0;
    hasScrollbar = false;
    selectionStart = -1;
    selectionEnd = -1;
    hasStoredSelection = false;
    for (int i = 0; i < 5; i++) {
        typeFilters[i] = true;
    }
    clear();
}

void OutputWindow::clear() {
    buf.clear();
    logs.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);
    needsRebuild = false;
}

void OutputWindow::addLog(LogType type, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char temp[4096];
    vsnprintf(temp, IM_ARRAYSIZE(temp), fmt, args);
    va_end(args);

    addLog(type, std::string(temp));
}

void OutputWindow::addLog(LogType type, const std::string& message) {
    // Store the raw log entry
    LogData logEntry{type, message, static_cast<float>(ImGui::GetTime())};
    logs.push_back(logEntry);

    // Mark that we need to rebuild
    needsRebuild = true;

    // Basic formatting for initial display
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

    if (autoScrollLocked && autoScroll) {
        scrollStartCount = 0;
    }
}

std::string getTypePrefix(LogType type) {
    switch(type) {
        case LogType::Error: return "[Error] ";
        case LogType::Success: return "[Success] ";
        case LogType::Info: return "[Info] ";
        case LogType::Warning: return "[Warning] ";
        case LogType::Build: return "[Build] ";
        default: return "[Unknown] ";
    }
}

void OutputWindow::rebuildBuffer() {
    if (!ImGui::GetCurrentContext() || !ImGui::GetCurrentWindow()) {
        return;
    }

    float wrapWidth = ImGui::GetContentRegionAvail().x;
    if (wrapWidth <= 0) return;

    if (hasScrollbar) {
        wrapWidth -= ImGui::GetStyle().ScrollbarSize;
    }
    wrapWidth -= menuWidth - ImGui::GetStyle().ItemSpacing.x;

    buf.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);

    ImFont* font = ImGui::GetFont();
    float fontSize = ImGui::GetFontSize();

    for (size_t logIndex = 0; logIndex < logs.size(); ++logIndex) {
        const auto& log = logs[logIndex];
        std::string prefix = getTypePrefix(log.type);
        float prefixWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

        bool typeAllowed = false;
        switch(log.type) {
            case LogType::Info: typeAllowed = typeFilters[0]; break;
            case LogType::Warning: typeAllowed = typeFilters[1]; break;
            case LogType::Error: typeAllowed = typeFilters[2]; break;
            case LogType::Success: typeAllowed = typeFilters[3]; break;
            case LogType::Build: typeAllowed = typeFilters[4]; break;
        }

        if (log.message.empty()) {
            if (typeAllowed && (!filter.IsActive() || filter.PassFilter((prefix + "\n").c_str()))) {
                buf.append(prefix.c_str());
                if (logIndex < logs.size() - 1) {
                    buf.append("\n");
                }
                lineOffsets.push_back(buf.size());
            }
            continue;
        }

        std::string indentation(prefix.length(), ' ');
        float indentWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, indentation.c_str()).x;

        std::string currentLine = prefix;
        float currentLineWidth = prefixWidth;
        bool isFirstLine = true;

        for (size_t i = 0; i < log.message.length(); ++i) {
            char currentChar = log.message[i];
            float charWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, std::string(1, currentChar).c_str()).x;

            if (!isFirstLine && currentLine.empty()) {
                currentLine = indentation;
                currentLineWidth = indentWidth;
            }

            if (currentLineWidth + charWidth > wrapWidth && !currentLine.empty() && currentChar != '\n') {
                if (typeAllowed && (!filter.IsActive() || filter.PassFilter(currentLine.c_str()))) {
                    buf.append(currentLine.c_str());
                    buf.append("\n");
                    lineOffsets.push_back(buf.size());
                }
                currentLine = indentation + currentChar;
                currentLineWidth = indentWidth + charWidth;
                isFirstLine = false;
            } else {
                currentLine += currentChar;
                currentLineWidth += charWidth;
            }

            if (currentChar == '\n') {
                if (typeAllowed && (!filter.IsActive() || filter.PassFilter(currentLine.c_str()))) {
                    buf.append(currentLine.c_str());
                    if (i < log.message.length() - 1 || logIndex < logs.size() - 1) {
                        buf.append("\n");
                    }
                    lineOffsets.push_back(buf.size());
                }
                currentLine.clear();
                currentLineWidth = 0;
                isFirstLine = false;
            }
        }

        if (!currentLine.empty()) {
            if (typeAllowed && (!filter.IsActive() || filter.PassFilter(currentLine.c_str()))) {
                buf.append(currentLine.c_str());
                if (logIndex < logs.size() - 1) {
                    buf.append("\n");
                }
                lineOffsets.push_back(buf.size());
            }
        }
    }
}

void OutputWindow::show() {
    if (!ImGui::Begin("Output")) {
        ImGui::End();
        return;
    }

    // Calculate dimensions for the vertical menu and main content
    menuWidth = ImGui::CalcTextSize(ICON_FA_LOCK).x + ImGui::GetStyle().ItemSpacing.x * 2 + ImGui::GetStyle().FramePadding.x * 2;
    const ImVec2 windowSize = ImGui::GetContentRegionAvail();

    static float lastContentWidth = 0.0f;
    float currentContentWidth = ImGui::GetContentRegionAvail().x;

    static bool lastHasScrollbar = false;
    bool currentlastScrollbar = hasScrollbar;

    // Check if we need to rebuild the buffer
    if (needsRebuild || std::abs(lastContentWidth - currentContentWidth) > 1.0f || lastHasScrollbar != currentlastScrollbar) {
        lastContentWidth = currentContentWidth;
        lastHasScrollbar = currentlastScrollbar;
        if (currentContentWidth > 0) {  // Only rebuild if we have a valid size
            rebuildBuffer();
            needsRebuild = false;
        }
    }

    // Begin vertical menu
    ImGui::BeginChild("VerticalMenu", ImVec2(menuWidth, windowSize.y), true);

    // Lock button (lock/unlock icon) controlling autoScroll
    if (ImGui::Button(autoScrollLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN, ImVec2(ImGui::CalcTextSize(ICON_FA_LOCK).x + ImGui::GetStyle().FramePadding.x * 2, 0.0f))) {
        autoScrollLocked = !autoScrollLocked;
        if (autoScrollLocked) {
            scrollStartCount = 0;
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(autoScrollLocked ? "Disable Auto-scroll" : "Enable Auto-scroll");
    }

    // Filter button (filter icon)
    bool anyFilterDisabled = false;
    for (int i = 0; i < 5; i++) {
        if (!typeFilters[i]) {
            anyFilterDisabled = true;
            break;
        }
    }
    if (anyFilterDisabled || filter.IsActive()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.5f, 1.0f));
    }
    if (ImGui::Button(ICON_FA_FILTER)) {
        ImGui::OpenPopup("FilterPopup");
    }
    if (anyFilterDisabled || filter.IsActive()) {
        ImGui::PopStyleColor();
    }
    if (ImGui::BeginPopup("FilterPopup")) {
        bool filterChanged = false;

        // Type filters
        const char* filterNames[] = {"Info", "Warning", "Error", "Success", "Build"};
        for (int i = 0; i < 5; i++) {
            if (ImGui::Checkbox(filterNames[i], &typeFilters[i])) {
                filterChanged = true;
            }
        }

        ImGui::Separator();

        // Text filter
        if (filter.Draw("##filter", 200.0f)) {
            needsRebuild = true;
        }

        if (filterChanged) {
            needsRebuild = true;
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
        ImGui::SetTooltip("Clear output");
    }

    ImGui::EndChild();

    // Main content area
    ImGui::SameLine(menuWidth);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImGui::InputTextMultiline("##output", (char*)buf.begin(), buf.size() + 1, 
        ImVec2(-FLT_MIN, -FLT_MIN), 
        ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll);

    // Store selection before context menu opens
    if (ImGui::IsItemFocused()) {
        ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetID("##output"));
        if (state && state->HasSelection()) {
            selectionStart = state->GetSelectionStart();
            selectionEnd = state->GetSelectionEnd();
            hasStoredSelection = true;
        }
    }

    // Context menu for copying selected text
    if (ImGui::BeginPopupContextItem("##output_context")) {
        if (ImGui::MenuItem(ICON_FA_COPY"  Copy", NULL, false, hasStoredSelection)) {
            if (hasStoredSelection) {
                int select_start = selectionStart;
                int select_end = selectionEnd;
                if (select_start > select_end)
                    std::swap(select_start, select_end);

                std::string selected_text(buf.begin() + select_start, buf.begin() + select_end);
                ImGui::SetClipboardText(selected_text.c_str());
            }
        }
        ImGui::EndPopup();
    }
    else {
        // Reset stored selection when context menu is closed
        if (!ImGui::IsPopupOpen("##output_context")) {
            hasStoredSelection = false;
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // Handle auto-scrolling
    ImGuiContext& g = *GImGui;
    const char* child_window_name = NULL;
    ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X", g.CurrentWindow->Name, "##output", ImGui::GetID("##output"));
    if (ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name)){
        hasScrollbar = child_window->ScrollbarY;
        if (child_window->ScrollMax.y > 0.0f) {
            if (child_window->Scroll.y < child_window->ScrollMax.y) {
                autoScroll = false;
            } else {
                autoScroll = true;
            }

            if (autoScrollLocked) {
                if (scrollStartCount < 8) { // keep scroll in 8 frames
                    scrollStartCount++;
                    ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
                }
            }
        }
    }

    ImGui::End();
}