#include "Console.h"
#include "external/IconsFontAwesome6.h"
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace Supernova::Editor;

Console::Console() : autoScroll(true) {
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
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timeStr;
    timeStr << std::put_time(&tm, "%H:%M:%S");

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
    }

    //std::string formattedMessage = "[" + timeStr.str() + "] [" + typeStr + "] " + message + "\n";
    std::string formattedMessage = "[" + typeStr + "] " + message + "\n";

    // Add to ImGui buffer
    int oldSize = buf.size();
    buf.append(formattedMessage.c_str());
    for (int newSize = buf.size(); oldSize < newSize; oldSize++)
        if (buf[oldSize] == '\n')
            lineOffsets.push_back(oldSize + 1);
}

void Console::show() {
    if (!ImGui::Begin("Console")) {
        ImGui::End();
        return;
    }

    //if (ImGui::BeginMenuBar()) {
    //    if (ImGui::BeginMenu("Options")) {
    //        ImGui::Checkbox("Auto-scroll", &autoScroll);
    //        ImGui::EndMenu();
    //    }
    //    ImGui::EndMenuBar();
    //}

    // Buttons
    //if (ImGui::Button("Clear")) clear();
    //ImGui::SameLine();
    //if (ImGui::Button("Copy")) ImGui::LogToClipboard();
    //ImGui::SameLine();
    //filter.Draw("Filter", -100.0f);

    //ImGui::Separator();

    // Display content
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    if (filter.IsActive()) {
        const char* bufStart = buf.begin();
        const char* bufEnd = buf.end();

        for (int lineNo = 0; lineNo < lineOffsets.Size; lineNo++) {
            const char* lineStart = bufStart + lineOffsets[lineNo];
            const char* lineEnd = (lineNo + 1 < lineOffsets.Size) ? 
                (bufStart + lineOffsets[lineNo + 1] - 1) : bufEnd;

            if (filter.PassFilter(lineStart, lineEnd)) {
                // Set color based on log type
                if (strstr(lineStart, "[Error]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Success]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Warning]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Info]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                ImGui::TextUnformatted(lineStart, lineEnd);
                ImGui::PopStyleColor();
            }
        }
    } else {
        ImGuiListClipper clipper;
        clipper.Begin(lineOffsets.Size);
        while (clipper.Step()) {
            for (int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; lineNo++) {
                const char* lineStart = buf.begin() + lineOffsets[lineNo];
                const char* lineEnd = (lineNo + 1 < lineOffsets.Size) ? 
                    (buf.begin() + lineOffsets[lineNo + 1] - 1) : buf.end();

                // Set color based on log type
                if (strstr(lineStart, "[Error]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Success]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Warning]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.4f, 1.0f));
                else if (strstr(lineStart, "[Info]")) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                ImGui::TextUnformatted(lineStart, lineEnd);
                ImGui::PopStyleColor();
            }
        }
    }

    ImGui::PopStyleVar();

    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}