#ifndef LAYOUTCONSOLE_H
#define LAYOUTCONSOLE_H

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

    class Console {
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

        void rebuildBuffer();

    public:
        Console();
        void clear();
        void addLog(LogType type, const char* fmt, ...);
        void addLog(LogType type, const std::string& message);
        void show();
    };
}

#endif /* LAYOUTCONSOLE_H */