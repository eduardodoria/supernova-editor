#include "OutputWindow.h"
#include "external/IconsFontAwesome6.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "imgui_internal.h"

using namespace Supernova::Editor;

static ImVec4 GetLogTypeColor(LogType type) {
    // Tune to taste
    switch (type) {
        case LogType::Info:    return ImVec4(0.90f, 0.90f, 0.95f, 1.00f); // light gray-blue
        case LogType::Warning: return ImVec4(1.00f, 0.85f, 0.40f, 1.00f); // amber
        case LogType::Error:   return ImVec4(1.00f, 0.45f, 0.45f, 1.00f); // red
        case LogType::Success: return ImVec4(0.55f, 1.00f, 0.55f, 1.00f); // green
        case LogType::Build:   return ImVec4(0.60f, 0.85f, 1.00f, 1.00f); // sky
        default:               return ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    }
}

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
    isSelecting = false;
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
    lineTypes.clear();
    needsRebuild = false;
    selectionStart = selectionEnd = -1;
    hasStoredSelection = false;
    isSelecting = false;
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

    // Basic formatting for initial display (append quickly for incremental feel)
    std::string typeStr;
    switch(type) {
        case LogType::Error: typeStr = "Error"; break;
        case LogType::Success: typeStr = "Success"; break;
        case LogType::Info: typeStr = "Info"; break;
        case LogType::Warning: typeStr = "Warning"; break;
        case LogType::Build: typeStr = "Build"; break;
    }
    std::string formattedMessage = "[" + typeStr + "] " + message + "\n";

    // Append raw for immediate feedback (will be rebuilt and recolored on next frame)
    int oldSize = buf.size();
    buf.append(formattedMessage.c_str());
    for (int newSize = buf.size(); oldSize < newSize; oldSize++)
        if (buf[oldSize] == '\n')
            lineOffsets.push_back(oldSize + 1);

    if (autoScrollLocked && autoScroll) {
        scrollStartCount = 0;
    }
}

static std::string getTypePrefixString(LogType type) {
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
    lineTypes.clear();

    ImFont* font = ImGui::GetFont();
    float fontSize = ImGui::GetFontSize();

    for (size_t logIndex = 0; logIndex < logs.size(); ++logIndex) {
        const auto& log = logs[logIndex];
        std::string prefix = getTypePrefixString(log.type);
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
                lineTypes.push_back(log.type);
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
                    lineTypes.push_back(log.type);
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
                    lineTypes.push_back(log.type);
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
                lineTypes.push_back(log.type);
            }
        }
    }

    // Reset selection if buffer changed drastically (optional; keep it if you want)
    if (selectionStart >= buf.size() || selectionEnd > buf.size()) {
        selectionStart = selectionEnd = -1;
        hasStoredSelection = false;
        isSelecting = false;
    }
}

// Ensure index is at a UTF-8 code-point boundary inside [0, buf.size()]
int OutputWindow::clampIndexToCodepointBoundary(int idx) const {
    idx = std::max(0, std::min(idx, (int)buf.size()));
    // Move backward until at boundary (0xxxxxxx or 11xxxxxx start)
    while (idx > 0) {
        unsigned char c = (unsigned char)buf[idx];
        if ((c & 0xC0) != 0x80) break; // not a continuation byte
        idx--;
    }
    return idx;
}

bool OutputWindow::isWordChar(unsigned int cp) {
    // Consider letters, digits, underscore as word chars. Others are delimiters.
    if (cp == '_') return true;
    if (cp >= '0' && cp <= '9') return true;
    if (cp >= 'A' && cp <= 'Z') return true;
    if (cp >= 'a' && cp <= 'z') return true;
    // Basic non-ASCII letters could be treated as word chars. Heuristic:
    if (cp > 127 && (ImCharIsBlankW((ImWchar)cp) == false)) return true;
    return false;
}

void OutputWindow::selectWordAt(int bufferIndex) {
    if (buf.empty()) return;
    int n = (int)buf.size();
    int idx = clampIndexToCodepointBoundary(bufferIndex);
    if (idx >= n) idx = n - 1;
    if (idx < 0) return;

    // Determine codepoint at idx (if idx points to end of a character, move back one)
    int cp_start = idx;
    // If idx equals n, back up
    if (cp_start >= n) cp_start = n - 1;
    // Ensure cp_start at boundary
    cp_start = clampIndexToCodepointBoundary(cp_start);

    // Decode codepoint at cp_start
    const char* begin = buf.begin();
    const char* s = begin + cp_start;
    const char* e = begin + n;
    unsigned int cp = 0;
    int len = ImTextCharFromUtf8(&cp, s, e);
    if (len <= 0) {
        // Fallback: select single char
        selectionStart = cp_start;
        selectionEnd = std::min(cp_start + 1, n);
        hasStoredSelection = selectionEnd > selectionStart;
        return;
    }

    bool wordChar = isWordChar(cp);

    // Expand left
    int left = cp_start;
    while (left > 0) {
        int prev = left;
        // step one codepoint back
        do { prev--; } while (prev > 0 && ((unsigned char)buf[prev] & 0xC0) == 0x80);
        unsigned int cp2 = 0;
        int l2 = ImTextCharFromUtf8(&cp2, begin + prev, begin + n);
        if (l2 <= 0) break;
        if (isWordChar(cp2) != wordChar) break;
        left = prev;
    }

    // Expand right
    int right = cp_start + len;
    while (right < n) {
        unsigned int cp2 = 0;
        int l2 = ImTextCharFromUtf8(&cp2, begin + right, begin + n);
        if (l2 <= 0) break;
        if (isWordChar(cp2) != wordChar) break;
        right += l2;
    }

    selectionStart = left;
    selectionEnd = right;
    hasStoredSelection = selectionEnd > selectionStart;
}

void OutputWindow::selectLineAt(int bufferIndex) {
    if (buf.empty()) return;
    int n = (int)buf.size();
    int idx = std::max(0, std::min(bufferIndex, n));

    // Find start of line (after previous '\n' or 0)
    int start = idx;
    while (start > 0 && buf[start - 1] != '\n') start--;

    // Find end of line (position of '\n' or n)
    int end = idx;
    while (end < n && buf[end] != '\n') end++;

    selectionStart = start;
    selectionEnd = end; // exclude '\n' to match common editors
    hasStoredSelection = selectionEnd > selectionStart;
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
        if (currentContentWidth > 0) {
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
    ImGui::SetNextWindowSize(ImVec2(200.0f, 0.0f));
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

        // Text filter with Structure-like search layout
        float inputHeight = ImGui::GetFrameHeight();
        ImVec2 buttonSize = ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS);
        buttonSize.x += ImGui::GetStyle().FramePadding.x * 2.0f;
        buttonSize.y = inputHeight;

        ImGui::BeginGroup();

        // ESC clears filter
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) && ImGui::IsWindowFocused()) {
            filter.InputBuf[0] = '\0';
            filter.Build();           // IMPORTANT: rebuild internal tokens
            needsRebuild = true;
        }

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - buttonSize.x);
        if (ImGui::InputText("##output_search", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf))) {
            filter.Build();           // IMPORTANT: keep filter logic in sync
            needsRebuild = true;
        }
        ImGui::PopItemWidth();

        ImGui::SameLine(0, 0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
        if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS)) {
            if (filter.InputBuf[0] != '\0') {
                filter.InputBuf[0] = '\0';
                filter.Build();
                needsRebuild = true;
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clear");
        }
        ImGui::PopStyleColor(3);

        ImGui::EndGroup();

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

    // Custom colored, selectable log viewer
    ImGui::BeginChild("##output", ImVec2(-FLT_MIN, -FLT_MIN), false, ImGuiWindowFlags_NoNav);

    // Cursor: I-beam over content, default over scrollbar
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        // Compute content rect in screen space (excludes scrollbars)
        ImVec2 content_min = ImGui::GetWindowContentRegionMin();
        ImVec2 content_max = ImGui::GetWindowContentRegionMax();
        ImVec2 win_pos     = ImGui::GetWindowPos();
        ImRect content_rect(
            ImVec2(win_pos.x + content_min.x, win_pos.y + content_min.y),
            ImVec2(win_pos.x + content_max.x, win_pos.y + content_max.y)
        );

        // If mouse is inside the content rect, show text cursor.
        // Otherwise (e.g., over scrollbar), leave default cursor.
        if (content_rect.Contains(ImGui::GetMousePos())) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        }
    }

    // Keyboard shortcuts
    const bool winFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
    const ImGuiIO& io = ImGui::GetIO();
    if (winFocused) {
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false)) {
            // Select all
            selectionStart = 0;
            selectionEnd = buf.size();
            hasStoredSelection = (selectionEnd > selectionStart);
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C, false)) {
            // Copy selection
            if (hasStoredSelection && selectionStart >= 0 && selectionEnd > selectionStart && selectionEnd <= buf.size()) {
                std::string selected_text(buf.begin() + selectionStart, buf.begin() + selectionEnd);
                ImGui::SetClipboardText(selected_text.c_str());
            }
        }
    }

    // Compute measurements
    ImFont* font = ImGui::GetFont();
    const float fontSize = ImGui::GetFontSize();
    const float lineHeight = ImGui::GetTextLineHeight();
    const int totalLines = (lineOffsets.Size > 0) ? (lineOffsets.Size - 1) : 0;
    if ((int)lineTypes.size() != totalLines) {
        // Safety: if out of sync, rebuild.
        rebuildBuffer();
    }

    // Helper lambdas
    auto lineStartIndex = [&](int line)->int { return (line >= 0 && line < lineOffsets.Size) ? lineOffsets[line] : 0; };
    auto lineEndIndexExcl = [&](int line)->int {
        if (line < 0 || line + 1 >= lineOffsets.Size) return buf.size();
        int end = lineOffsets[line + 1];
        // Exclude the newline if present
        if (end > 0 && end <= buf.size() && buf[end - 1] == '\n') end -= 1;
        return end;
    };

    // Map mouse to buffer index (character-precise)
    auto bufferIndexFromMouse = [&](const ImVec2& originScreen)->int {
        // Convert mouse position to local content coordinates inside this child
        float localX = ImGui::GetMousePos().x - originScreen.x;
        float localY = ImGui::GetMousePos().y - originScreen.y + ImGui::GetScrollY();
        int line = (int)std::floor(localY / lineHeight);
        line = std::max(0, std::min(line, totalLines - 1));

        int ls = lineStartIndex(line);
        int le = lineEndIndexExcl(line);

        // Clamp
        if (ls >= le) return ls;

        // Find character position in line by accumulating width
        const char* s = buf.begin() + ls;
        const char* e = buf.begin() + le;

        float x = 0.0f;
        int idx = ls;
        for (const char* p = s; p < e; ) {
            // ImGui uses UTF-8; advance by one code-point
            unsigned int c;
            int c_len = ImTextCharFromUtf8(&c, p, e);
            if (c_len <= 0) break;

            float cw = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, p, p + c_len).x;
            if (localX < x + cw * 0.5f) {
                return idx;
            }
            x += cw;
            p += c_len;
            idx += c_len;
        }
        return le; // past end of line
    };

    // Handle mouse selection (click/drag/double/triple)
    ImVec2 originScreen = ImGui::GetCursorScreenPos();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        // Multi-click detection via Dear ImGui
        // 1 click: start drag-selection
        // 2 clicks: word selection
        // 3 clicks: line selection
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            int idx = bufferIndexFromMouse(originScreen);
            int clicks = ImGui::GetIO().MouseClickedCount[ImGuiMouseButton_Left];

            if (clicks >= 3) {
                selectLineAt(idx);
                isSelecting = false; // finalize
            } else if (clicks == 2) {
                selectWordAt(idx);
                isSelecting = false; // finalize
            } else {
                // single click -> start drag selection
                selectionStart = selectionEnd = idx;
                hasStoredSelection = true;
                isSelecting = true;
            }
        } else if (isSelecting && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            int idx = bufferIndexFromMouse(originScreen);
            selectionEnd = idx;
            hasStoredSelection = (selectionStart != selectionEnd);
        } else if (isSelecting && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            isSelecting = false;
        }
    }

    // Context menu for copying selected text
    if (ImGui::BeginPopupContextWindow("##output_context", ImGuiPopupFlags_MouseButtonRight)) {
        bool canCopy = hasStoredSelection && selectionStart >= 0 && selectionEnd > selectionStart && selectionEnd <= buf.size();
        if (ImGui::MenuItem(ICON_FA_COPY "  Copy", NULL, false, canCopy)) {
            if (canCopy) {
                int a = selectionStart, b = selectionEnd;
                if (a > b) std::swap(a, b);
                std::string selected_text(buf.begin() + a, buf.begin() + b);
                ImGui::SetClipboardText(selected_text.c_str());
            }
        }
        if (ImGui::MenuItem("Select All")) {
            selectionStart = 0;
            selectionEnd = buf.size();
            hasStoredSelection = selectionEnd > selectionStart;
        }
        ImGui::EndPopup();
    }

    // Draw with clipping (fast for large logs)
    ImVec2 baseCursorPos = ImGui::GetCursorPos();
    ImGuiListClipper clipper;
    clipper.Begin(totalLines, lineHeight);

    const ImU32 selBgCol = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg]);

    // Ensure selection indices are ordered
    int selA = selectionStart, selB = selectionEnd;
    if (selA > selB) std::swap(selA, selB);

    while (clipper.Step()) {
        ImGui::SetCursorPosY(baseCursorPos.y + clipper.DisplayStart * lineHeight);

        for (int line = clipper.DisplayStart; line < clipper.DisplayEnd; ++line) {
            int ls = lineStartIndex(line);
            int le = lineEndIndexExcl(line);

            ImVec2 linePos = ImGui::GetCursorScreenPos();

            // Selection highlight for the portion of this line that is selected
            if (hasStoredSelection && selA >= 0 && selB >= 0 && selB > selA) {
                int hlStart = std::max(ls, selA);
                int hlEnd   = std::min(le, selB);
                if (hlStart < hlEnd) {
                    // Compute x offsets by measuring text width
                    const char* s0 = buf.begin() + ls;
                    const char* s1 = buf.begin() + hlStart;
                    const char* s2 = buf.begin() + hlEnd;

                    float x0 = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, s0, s1).x;
                    float x1 = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, s1, s2).x;

                    ImGui::GetWindowDrawList()->AddRectFilled(
                        ImVec2(linePos.x + x0, linePos.y),
                        ImVec2(linePos.x + x0 + x1, linePos.y + lineHeight),
                        selBgCol);
                }
            }

            // Colored text by type
            ImVec4 color = (line >= 0 && line < (int)lineTypes.size()) ? GetLogTypeColor(lineTypes[line])
                                                                       : ImVec4(1,1,1,1);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            if (ls < le) {
                ImGui::TextUnformatted(buf.begin() + ls, buf.begin() + le);
            } else {
                ImGui::TextUnformatted(""); // empty line
            }
            ImGui::PopStyleColor();
        }
    }
    clipper.End();

    // Auto-scroll handling (inside child)
    {
        float maxY = ImGui::GetScrollMaxY();
        float curY = ImGui::GetScrollY();
        hasScrollbar = maxY > 0.0f;

        if (maxY > 0.0f) {
            autoScroll = (curY >= maxY - 1.0f);
            if (autoScrollLocked) {
                if (scrollStartCount < 8) { // keep scroll pinned for a few frames
                    scrollStartCount++;
                    ImGui::SetScrollY(maxY);
                }
            }
        }
    }

    ImGui::EndChild();

    ImGui::End();
}