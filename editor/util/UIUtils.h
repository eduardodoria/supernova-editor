#pragma once

#include <string>
#include "imgui.h"

namespace Supernova::Editor {
    class UIUtils {
    public:
        // Draw a search input field with a magnifying glass icon button
        // Returns true if the input text changed
        static bool searchInput(const char* id, std::string hint, char* buffer, size_t bufferSize, bool autoFocus = false, bool* matchCase = nullptr);
    };
}