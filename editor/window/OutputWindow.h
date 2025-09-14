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
        ImVector<int> lineOffsets;           // Start index of each line in buf (size = lines + 1)
        std::vector<LogData> logs;
        std::vector<LogType> lineTypes;      // One per displayed line

        bool autoScrollLocked;
        bool autoScroll;
        bool needsRebuild;
        unsigned int scrollStartCount;
        float menuWidth;
        bool hasScrollbar;

        // Character-level selection (global indices into buf)
        int selectionStart;                  // inclusive
        int selectionEnd;                    // exclusive
        bool hasStoredSelection;
        bool isSelecting;                    // dragging selection?

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