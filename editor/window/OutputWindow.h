#pragma once

#include <vector>
#include <string>
#include "imgui.h"

namespace Supernova::Editor {
    enum class LogType {
        Info,
        Warning,
        Error,
        Success,
        Build
    };

    struct LogData {
        LogType type;
        std::string message;
        float timestamp;
    };

    class OutputWindow {
    private:
        ImGuiTextBuffer buf;
        ImGuiTextFilter filter;
        ImVector<int> lineOffsets;
        std::vector<LogData> logs;
        bool autoScrollLocked;
        bool autoScroll;
        bool needsRebuild;
        unsigned int scrollStartCount;
        float menuWidth;
        bool hasScrollbar;
        int selectionStart;
        int selectionEnd;
        bool hasStoredSelection;
        bool typeFilters[5];

        void rebuildBuffer();

    public:
        OutputWindow();
        void clear();
        void addLog(LogType type, const char* fmt, ...);
        void addLog(LogType type, const std::string& message);
        void show();
    };
}