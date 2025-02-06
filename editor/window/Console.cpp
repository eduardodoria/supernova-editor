// Console.cpp
#include "Console.h"
#include "external/IconsFontAwesome6.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "imgui_internal.h"

using namespace Supernova::Editor;

Console::Console() {
    autoScroll = true;
    autoScrollLocked = true;
    scrollStartCount = 0;
    needsRebuild = false;
    clear();
}

void Console::clear() {
    buf.clear();
    logs.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);
    needsRebuild = false;
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

void Console::rebuildBuffer() {
    // If no ImGui context or window, return early
    if (!ImGui::GetCurrentContext() || !ImGui::GetCurrentWindow()) {
        return;
    }

    float wrapWidth = ImGui::GetContentRegionAvail().x;
    if (wrapWidth <= 0) return; // Skip if width is invalid

    buf.clear();
    lineOffsets.clear();
    lineOffsets.push_back(0);

    ImFont* font = ImGui::GetFont();

    for (const auto& log : logs) {
        std::string prefix = getTypePrefix(log.type);
        float prefixWidth = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, prefix.c_str()).x;

        // Split message into words
        std::istringstream iss(log.message);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // Handle empty message case
        if (words.empty()) {
            if (!filter.IsActive() || filter.PassFilter((prefix + "\n").c_str())) {
                buf.append((prefix + "\n").c_str());
                lineOffsets.push_back(buf.size());
            }
            continue;
        }

        std::string currentLine = prefix;
        float currentLineWidth = prefixWidth;
        std::string indentation(prefix.length(), ' ');
        bool firstLine = true;

        for (size_t i = 0; i < words.size(); ++i) {
            const std::string& word = words[i];
            float wordWidth = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, (word + " ").c_str()).x;

            bool isLastWord = (i == words.size() - 1);

            if (!firstLine && currentLineWidth == prefixWidth) {
                // Start of a new line after wrapping
                currentLine += indentation;
                currentLineWidth += font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, indentation.c_str()).x;
            }

            if (currentLineWidth + wordWidth > wrapWidth && !firstLine) {
                // Wrap to new line
                if (!filter.IsActive() || filter.PassFilter(currentLine.c_str())) {
                    buf.append((currentLine + "\n").c_str());
                    lineOffsets.push_back(buf.size());
                }
                currentLine = indentation + word;
                currentLineWidth = prefixWidth + wordWidth;
            } else {
                // Add word to current line
                if (!firstLine || !currentLine.empty()) {
                    currentLine += " ";
                    currentLineWidth += font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, " ").x;
                }
                currentLine += word;
                currentLineWidth += wordWidth;
            }

            if (isLastWord) {
                if (!filter.IsActive() || filter.PassFilter(currentLine.c_str())) {
                    buf.append((currentLine + "\n").c_str());
                    lineOffsets.push_back(buf.size());
                }
            }

            firstLine = false;
        }
    }
}

void Console::show() {
    if (!ImGui::Begin("Console")) {
        ImGui::End();
        return;
    }

    // Calculate dimensions for the vertical menu and main content
    const float menuWidth = ImGui::CalcTextSize(ICON_FA_LOCK).x + ImGui::GetStyle().ItemSpacing.x * 2 + ImGui::GetStyle().FramePadding.x * 2;
    const ImVec2 windowSize = ImGui::GetContentRegionAvail();

    static float lastContentWidth = 0.0f;
    float currentContentWidth = ImGui::GetContentRegionAvail().x;

    // Check if we need to rebuild the buffer
    if (needsRebuild || std::abs(lastContentWidth - currentContentWidth) > 1.0f) {
        lastContentWidth = currentContentWidth;
        if (currentContentWidth > 0) {  // Only rebuild if we have a valid width
            rebuildBuffer();
            needsRebuild = false;
        }
    }

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
            needsRebuild = true;  // Need to rebuild when filter changes
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

    // Handle auto-scrolling
    ImGuiContext& g = *GImGui;
    const char* child_window_name = NULL;
    ImFormatStringToTempBuffer(&child_window_name, NULL, "%s/%s_%08X", g.CurrentWindow->Name, "##console", ImGui::GetID("##console"));
    ImGuiWindow* child_window = ImGui::FindWindowByName(child_window_name);

    if (child_window && child_window->ScrollMax.y > 0.0f) {
        if (child_window->Scroll.y < child_window->ScrollMax.y) {
            autoScroll = false;
        } else {
            autoScroll = true;
        }

        if (autoScrollLocked) {
            if (scrollStartCount < 3) { // keep scroll in 3 frames
                scrollStartCount++;
                ImGui::SetScrollY(child_window, child_window->ScrollMax.y);
            }
        }
    }

    ImGui::End();
}