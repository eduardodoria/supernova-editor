#include "SpriteSlicerToolDialog.h"
#include "external/IconsFontAwesome6.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <climits>

namespace Supernova {
namespace Editor {

void SpriteSlicerToolDialog::open(const Texture& previewTexture,
                                    int sheetWidth, int sheetHeight,
                                    std::function<void(const SliceResult&)> onApply,
                                    std::function<void()> onCancel) {
    m_isOpen = true;
    m_previewTexture = previewTexture;
    m_sheetWidth = sheetWidth;
    m_sheetHeight = sheetHeight;
    m_onApply = onApply;
    m_onCancel = onCancel;
    m_previewDirty = true;

    if (!m_previewTexture.empty()) {
        const int textureWidth = static_cast<int>(m_previewTexture.getWidth());
        const int textureHeight = static_cast<int>(m_previewTexture.getHeight());

        if (textureWidth > 0) {
            m_sheetWidth = textureWidth;
        }
        if (textureHeight > 0) {
            m_sheetHeight = textureHeight;
        }
    }

    // Smart defaults based on sheet size
    if (m_sheetWidth > 0 && m_sheetHeight > 0) {
        m_columns = std::max(1, m_sheetWidth / 32);
        m_rows = std::max(1, m_sheetHeight / 32);
        m_cellWidth = m_sheetWidth / m_columns;
        m_cellHeight = m_sheetHeight / m_rows;

        // Clamp to reasonable defaults
        if (m_columns > 16) m_columns = 4;
        if (m_rows > 16) m_rows = 4;
        m_cellWidth = m_sheetWidth / m_columns;
        m_cellHeight = m_sheetHeight / m_rows;
    }

    m_offsetX = 0;
    m_offsetY = 0;
    m_paddingX = 0;
    m_paddingY = 0;
    strncpy(m_prefixBuffer, "frame_", sizeof(m_prefixBuffer) - 1);
    m_prefixBuffer[sizeof(m_prefixBuffer) - 1] = '\0';
}

void SpriteSlicerToolDialog::generatePreview() {
    m_previewFrames.clear();

    if (m_sheetWidth <= 0 || m_sheetHeight <= 0) return;

    int cols, rows, cw, ch;

    if (m_sliceMode == SliceMode::GRID) {
        cols = std::max(1, m_columns);
        rows = std::max(1, m_rows);
        int availW = m_sheetWidth - m_offsetX;
        int availH = m_sheetHeight - m_offsetY;
        cw = std::max(1, (availW - m_paddingX * (cols - 1)) / cols);
        ch = std::max(1, (availH - m_paddingY * (rows - 1)) / rows);
    } else {
        cw = std::max(1, m_cellWidth);
        ch = std::max(1, m_cellHeight);
        int availW = m_sheetWidth - m_offsetX;
        int availH = m_sheetHeight - m_offsetY;
        cols = std::max(1, (availW + m_paddingX) / (cw + m_paddingX));
        rows = std::max(1, (availH + m_paddingY) / (ch + m_paddingY));
    }

    std::string prefix(m_prefixBuffer);
    int frameIndex = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            float fx = static_cast<float>(m_offsetX + c * (cw + m_paddingX));
            float fy = static_cast<float>(m_offsetY + r * (ch + m_paddingY));
            float fw = static_cast<float>(cw);
            float fh = static_cast<float>(ch);

            // Clip to sheet bounds
            if (fx + fw > m_sheetWidth) fw = m_sheetWidth - fx;
            if (fy + fh > m_sheetHeight) fh = m_sheetHeight - fy;
            if (fw <= 0 || fh <= 0) continue;

            SpriteFrameData frame;
            frame.active = true;
            frame.name = prefix + std::to_string(frameIndex);
            frame.rect = Rect(fx, fy, fw, fh);

            m_previewFrames.push_back(frame);
            frameIndex++;

            if (frameIndex >= MAX_SPRITE_FRAMES) break;
        }
        if (frameIndex >= MAX_SPRITE_FRAMES) break;
    }

    m_previewDirty = false;
}

void SpriteSlicerToolDialog::show() {
    if (!m_isOpen) return;

    const char* popupTitle = ICON_FA_GRIP " Sprite Slicer Tool##SpriteSlicerToolModal";

    ImGui::OpenPopup(popupTitle);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_Once);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_Modal;

    if (!ImGui::BeginPopupModal(popupTitle, &m_isOpen, flags)) {
        if (m_isOpen) {
            m_isOpen = false;
            if (m_onCancel) m_onCancel();
        }
        return;
    }

    auto beginInputTable = [](const char* id, float firstColumnWidth = 130.0f) {
        if (ImGui::BeginTable(id, 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, firstColumnWidth);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }

        return false;
    };

    auto beginInputRow = [](const char* label, float inputWidth = 0.0f) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(inputWidth > 0.0f ? inputWidth : 8.0f * ImGui::GetFontSize());
    };

    ImGui::Separator();

    if (m_sheetWidth > 0 && m_sheetHeight > 0) {
        ImGui::Text("Sheet size: %d x %d", m_sheetWidth, m_sheetHeight);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "Warning: Sheet dimensions unknown. Set Width/Height in sprite properties first.");
    }

    ImGui::Spacing();

    if (beginInputTable("SpriteSlicerSliceModeTable")) {
        beginInputRow("Slice Mode", 14.0f * ImGui::GetFontSize());
        if (ImGui::RadioButton("Grid (Columns x Rows)##SliceModeGrid", m_sliceMode == SliceMode::GRID)) {
            m_sliceMode = SliceMode::GRID;
            m_previewDirty = true;
        }
        //ImGui::SameLine();
        if (ImGui::RadioButton("Cell Size (W x H)##SliceModeCell", m_sliceMode == SliceMode::CELL_SIZE)) {
            m_sliceMode = SliceMode::CELL_SIZE;
            m_previewDirty = true;
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_sliceMode == SliceMode::GRID) {
        ImGui::SeparatorText("Grid");
        if (beginInputTable("SpriteSlicerGridTable")) {
            beginInputRow("Columns", 6.0f * ImGui::GetFontSize());
            if (ImGui::DragInt("##Columns", &m_columns, 0.1f, 1, 64)) {
                m_previewDirty = true;
            }

            beginInputRow("Rows", 6.0f * ImGui::GetFontSize());
            if (ImGui::DragInt("##Rows", &m_rows, 0.1f, 1, 64)) {
                m_previewDirty = true;
            }

            ImGui::EndTable();
        }
    } else {
        ImGui::SeparatorText("Cell Size");
        if (beginInputTable("SpriteSlicerCellSizeTable")) {
            beginInputRow("Cell Width", 6.0f * ImGui::GetFontSize());
            if (ImGui::DragInt("##CellWidth", &m_cellWidth, 1.0f, 1, m_sheetWidth > 0 ? m_sheetWidth : 4096)) {
                m_previewDirty = true;
            }

            beginInputRow("Cell Height", 6.0f * ImGui::GetFontSize());
            if (ImGui::DragInt("##CellHeight", &m_cellHeight, 1.0f, 1, m_sheetHeight > 0 ? m_sheetHeight : 4096)) {
                m_previewDirty = true;
            }

            ImGui::EndTable();
        }
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Offset & Padding");
    if (beginInputTable("SpriteSlicerOffsetPaddingTable")) {
        beginInputRow("Offset X", 6.0f * ImGui::GetFontSize());
        if (ImGui::DragInt("##OffsetX", &m_offsetX, 1.0f, 0, INT_MAX)) {
            m_previewDirty = true;
        }

        beginInputRow("Offset Y", 6.0f * ImGui::GetFontSize());
        if (ImGui::DragInt("##OffsetY", &m_offsetY, 1.0f, 0, INT_MAX)) {
            m_previewDirty = true;
        }

        beginInputRow("Padding X", 6.0f * ImGui::GetFontSize());
        if (ImGui::DragInt("##PaddingX", &m_paddingX, 1.0f, 0, INT_MAX)) {
            m_previewDirty = true;
        }

        beginInputRow("Padding Y", 6.0f * ImGui::GetFontSize());
        if (ImGui::DragInt("##PaddingY", &m_paddingY, 1.0f, 0, INT_MAX)) {
            m_previewDirty = true;
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Frame Naming");
    if (beginInputTable("SpriteSlicerFrameNamingTable")) {
        beginInputRow("Prefix", 10.0f * ImGui::GetFontSize());
        if (ImGui::InputText("##Prefix", m_prefixBuffer, sizeof(m_prefixBuffer))) {
            m_previewDirty = true;
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Generate preview
    if (m_previewDirty) {
        generatePreview();
    }

    // Visual preview
    ImGui::Text("Preview: %d frames", (int)m_previewFrames.size());

    if (m_sheetWidth > 0 && m_sheetHeight > 0) {
        auto drawPreview = [this](const ImVec2& canvasPos, const ImVec2& canvasSize, ImDrawList* drawList) {
            const float scale = std::min(canvasSize.x / static_cast<float>(m_sheetWidth),
                                         canvasSize.y / static_cast<float>(m_sheetHeight));
            const float previewW = m_sheetWidth * scale;
            const float previewH = m_sheetHeight * scale;
            const float offsetX = (canvasSize.x - previewW) * 0.5f;
            const float offsetY = (canvasSize.y - previewH) * 0.5f;

            ImVec2 imageMin(canvasPos.x + offsetX, canvasPos.y + offsetY);
            ImVec2 imageMax(imageMin.x + previewW, imageMin.y + previewH);

            drawList->AddRectFilled(canvasPos,
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                IM_COL32(40, 40, 40, 255));

            if (!m_previewTexture.empty() && m_previewTexture.getRender()) {
                drawList->AddImage(m_previewTexture.getRender()->getGLHandler(), imageMin, imageMax);
            }

            drawList->AddRect(imageMin, imageMax, IM_COL32(100, 100, 100, 255));

            ImU32 borderColor = IM_COL32(100, 200, 100, 220);
            for (size_t i = 0; i < m_previewFrames.size(); i++) {
                const SpriteFrameData& frame = m_previewFrames[i];
                float rx = imageMin.x + frame.rect.getX() * scale;
                float ry = imageMin.y + frame.rect.getY() * scale;
                float rw = frame.rect.getWidth() * scale;
                float rh = frame.rect.getHeight() * scale;

                drawList->AddRect(ImVec2(rx, ry), ImVec2(rx + rw, ry + rh), borderColor);

                if (rw > 16 && rh > 12) {
                    char indexStr[8];
                    snprintf(indexStr, sizeof(indexStr), "%d", (int)i);
                    ImVec2 textSize = ImGui::CalcTextSize(indexStr);
                    float textX = rx + (rw - textSize.x) * 0.5f;
                    float textY = ry + (rh - textSize.y) * 0.5f;
                    drawList->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 200), indexStr);
                }
            }

            drawList->AddRect(canvasPos,
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                IM_COL32(80, 80, 80, 255));
        };

        const ImVec2 compactCanvasSize(100.0f, 100.0f);
        ImVec2 compactCanvasPos = ImGui::GetCursorScreenPos();
        drawPreview(compactCanvasPos, compactCanvasSize, ImGui::GetWindowDrawList());
        ImGui::Dummy(compactCanvasSize);

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%d x %d", m_sheetWidth, m_sheetHeight);
            ImVec2 tooltipCanvasPos = ImGui::GetCursorScreenPos();
            ImVec2 tooltipCanvasSize(static_cast<float>(m_sheetWidth), static_cast<float>(m_sheetHeight));
            drawPreview(tooltipCanvasPos, tooltipCanvasSize, ImGui::GetWindowDrawList());
            ImGui::Dummy(tooltipCanvasSize);
            ImGui::EndTooltip();
        }
    }

    // Frame list summary
    if (!m_previewFrames.empty() && ImGui::TreeNode("Frame Details")) {
        ImGui::BeginChild("FrameList", ImVec2(0, 150), true);
        for (size_t i = 0; i < m_previewFrames.size(); i++) {
            const SpriteFrameData& frame = m_previewFrames[i];
            ImGui::Text("[%d] %s: (%.0f, %.0f, %.0f, %.0f)",
                (int)i, frame.name.c_str(),
                frame.rect.getX(), frame.rect.getY(),
                frame.rect.getWidth(), frame.rect.getHeight());
        }
        ImGui::EndChild();
        ImGui::TreePop();
    }

    if (m_previewFrames.size() >= MAX_SPRITE_FRAMES) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f),
            "Warning: Frame count capped at %d (MAX_SPRITE_FRAMES)", MAX_SPRITE_FRAMES);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    float windowWidth = ImGui::GetWindowSize().x;
    float buttonsWidth = 260;
    ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

    ImGui::BeginDisabled(m_previewFrames.empty());
    if (ImGui::Button("Apply", ImVec2(120, 0))) {
        if (m_onApply) {
            SliceResult result;
            result.frames = m_previewFrames;
            m_onApply(result);
        }
        m_isOpen = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        m_isOpen = false;
        if (m_onCancel) m_onCancel();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

} // namespace Editor
} // namespace Supernova
