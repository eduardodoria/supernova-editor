#ifndef LAYOUTCONSOLE_H
#define LAYOUTCONSOLE_H

#include <vector>
#include <string>
#include "imgui.h"

namespace Supernova::Editor {
    enum class LogType {
        Error,
        Success,
        Info,
        Warning
    };

    struct LogData {
        LogType type;
        std::string message;
        float time;
    };

    class Console {
    private:
        ImGuiTextBuffer buf;
        ImGuiTextFilter filter;
        ImVector<int> lineOffsets;    // Index to lines offset
        std::vector<LogData> logs;          // Store logs with their types
        bool autoScroll;      // Keep scrolling if already at the bottom

        // Helper method to rebuild the colored buffer
        void rebuildBuffer();

    public:
        Console();
        ~Console() = default;

        void show();
        void clear();
        void addLog(LogType type, const char* fmt, ...) IM_FMTARGS(3);
        void addLog(LogType type, const std::string& message);
    };
}

#endif /* LAYOUTCONSOLE_H */