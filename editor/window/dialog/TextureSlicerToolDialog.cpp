#include "TextureSlicerToolDialog.h"
#include "external/IconsFontAwesome6.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <climits>

namespace Supernova {
namespace Editor {

void TextureSlicerToolDialog::open(const Texture& previewTexture,
                                    int sheetWidth, int sheetHeight,
                                    std::function<void(const SliceResult&)> onApply,
                                    std::function<void()> onCancel) {
    m_isOpen = true;
    m_mode = Mode::SPRITE;
    m_previewTexture = previewTexture;
    m_sheetWidth = sheetWidth;
    m_sheetHeight = sheetHeight;
    m_onApply = onApply;
    m_onCancel = onCancel;
    m_previewDirty = true;

    if (!m_previewTexture.empty()) {
        int textureWidth = static_cast<int>(m_previewTexture.getWidth());
        int textureHeight = static_cast<int>(m_previewTexture.getHeight());

        // If texture data not loaded yet, force load to get dimensions
        if (textureWidth <= 0 || textureHeight <= 0) {
            m_previewTexture.load();
            textureWidth = static_cast<int>(m_previewTexture.getWidth());
            textureHeight = static_cast<int>(m_previewTexture.getHeight());
        }

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

    m_autoFillGrid = false;
}

void TextureSlicerToolDialog::openTileset(const Texture& previewTexture,
                                           int sheetWidth, int sheetHeight,
                                           unsigned int tilemapWidth, unsigned int tilemapHeight,
                                           std::function<void(const SliceResult&)> onApply,
                                           std::function<void()> onCancel) {
    // Initialize shared slicing state via the base open
    open(previewTexture, sheetWidth, sheetHeight, onApply, onCancel);

    // Override for tileset mode
    m_mode = Mode::TILESET;
    strncpy(m_prefixBuffer, "tile_", sizeof(m_prefixBuffer) - 1);
    m_prefixBuffer[sizeof(m_prefixBuffer) - 1] = '\0';

    // Auto-fill grid defaults based on tilemap dimensions
    m_autoFillGrid = false;
    m_fillPattern = FillPattern::SEQUENTIAL;
    m_singleRectId = 0;
    m_autoTileSize = true;

    if (tilemapWidth > 0 && tilemapHeight > 0) {
        m_tileWidth = static_cast<float>(m_cellWidth > 0 ? m_cellWidth : 32);
        m_tileHeight = static_cast<float>(m_cellHeight > 0 ? m_cellHeight : 32);
        m_gridColumns = std::max(1, static_cast<int>(tilemapWidth / m_tileWidth));
        m_gridRows = std::max(1, static_cast<int>(tilemapHeight / m_tileHeight));
    } else {
        m_tileWidth = 32.0f;
        m_tileHeight = 32.0f;
        m_gridColumns = 10;
        m_gridRows = 10;
    }
}

void TextureSlicerToolDialog::generatePreview() {
    m_previewRects.clear();

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
    int rectIndex = 0;

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

            SlicedRect slicedRect;
            slicedRect.name = prefix + std::to_string(rectIndex);
            slicedRect.rect = Rect(fx, fy, fw, fh);

            m_previewRects.push_back(slicedRect);
            rectIndex++;
        }
    }

    // Update auto tile size from cell size (tileset mode)
    if (m_mode == Mode::TILESET && m_autoTileSize) {
        m_tileWidth = static_cast<float>(cw);
        m_tileHeight = static_cast<float>(ch);
    }

    m_previewDirty = false;
}

TextureSlicerToolDialog::SliceResult TextureSlicerToolDialog::buildResult() {
    SliceResult result;
    result.rects = m_previewRects;
    result.hasTileGrid = m_autoFillGrid && m_mode == Mode::TILESET;

    if (result.hasTileGrid && !m_previewRects.empty()) {
        int gridCols = std::max(1, m_gridColumns);
        int gridRows = std::max(1, m_gridRows);
        float tw = std::max(1.0f, m_tileWidth);
        float th = std::max(1.0f, m_tileHeight);
        int numRects = static_cast<int>(m_previewRects.size());

        int tileIndex = 0;
        for (int r = 0; r < gridRows; r++) {
            for (int c = 0; c < gridCols; c++) {
                TileGridEntry tile;
                tile.name = "tile_" + std::to_string(tileIndex);

                if (m_fillPattern == FillPattern::SEQUENTIAL) {
                    tile.rectId = tileIndex % numRects;
                } else {
                    tile.rectId = std::clamp(m_singleRectId, 0, numRects - 1);
                }

                tile.position = Vector2(static_cast<float>(c) * tw, static_cast<float>(r) * th);
                tile.width = tw;
                tile.height = th;

                result.tiles.push_back(tile);
                tileIndex++;
            }
        }
    }

    return result;
}

void TextureSlicerToolDialog::show() {
    if (!m_isOpen) return;

    const bool isTileset = (m_mode == Mode::TILESET);
    const char* popupTitle = isTileset
        ? ICON_FA_TABLE_CELLS " Tileset Slicer Tool##TextureSlicerToolModal"
        : ICON_FA_GRIP " Sprite Slicer Tool##TextureSlicerToolModal";
    const char* rectLabel = isTileset ? "tile rects" : "frames";
    const char* namingLabel = isTileset ? "Rect Naming" : "Frame Naming";
    const char* detailsLabel = isTileset ? "Rect Details" : "Frame Details";

    ImGui::OpenPopup(popupTitle);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(isTileset ? 540.0f : 520.0f, 0), ImGuiCond_Once);

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
        ImGui::Text("%s size: %d x %d", isTileset ? "Texture" : "Sheet", m_sheetWidth, m_sheetHeight);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f),
            isTileset ? "Warning: Texture dimensions unknown. Assign a texture first."
                      : "Warning: Sheet dimensions unknown. Set Width/Height in sprite properties first.");
    }

    ImGui::Spacing();

    // --- Slice Mode ---
    if (beginInputTable("SlicerSliceModeTable")) {
        beginInputRow("Slice Mode", 14.0f * ImGui::GetFontSize());
        if (ImGui::RadioButton("Grid (Columns x Rows)##SliceModeGrid", m_sliceMode == SliceMode::GRID)) {
            m_sliceMode = SliceMode::GRID;
            m_previewDirty = true;
        }
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
        if (beginInputTable("SlicerGridTable")) {
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
        if (beginInputTable("SlicerCellSizeTable")) {
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
    if (beginInputTable("SlicerOffsetPaddingTable")) {
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
    ImGui::SeparatorText(namingLabel);
    if (beginInputTable("SlicerNamingTable")) {
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

    // --- Visual preview ---
    ImGui::Text("Preview: %d %s", (int)m_previewRects.size(), rectLabel);

    if (m_sheetWidth > 0 && m_sheetHeight > 0) {
        auto drawPreview = [this, isTileset](const ImVec2& canvasPos, const ImVec2& canvasSize, ImDrawList* drawList) {
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

            ImU32 borderColor = isTileset ? IM_COL32(80, 180, 220, 220) : IM_COL32(100, 200, 100, 220);
            for (size_t i = 0; i < m_previewRects.size(); i++) {
                const SlicedRect& slicedRect = m_previewRects[i];
                float rx = imageMin.x + slicedRect.rect.getX() * scale;
                float ry = imageMin.y + slicedRect.rect.getY() * scale;
                float rw = slicedRect.rect.getWidth() * scale;
                float rh = slicedRect.rect.getHeight() * scale;

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

    // Rect/frame list summary
    if (!m_previewRects.empty() && ImGui::TreeNode(detailsLabel)) {
        ImGui::BeginChild("RectList", ImVec2(0, isTileset ? 120.0f : 150.0f), true);
        for (size_t i = 0; i < m_previewRects.size(); i++) {
            const SlicedRect& slicedRect = m_previewRects[i];
            ImGui::Text("[%d] %s: (%.0f, %.0f, %.0f, %.0f)",
                (int)i, slicedRect.name.c_str(),
                slicedRect.rect.getX(), slicedRect.rect.getY(),
                slicedRect.rect.getWidth(), slicedRect.rect.getHeight());
        }
        ImGui::EndChild();
        ImGui::TreePop();
    }

    // --- Auto-fill Tiles Section (tileset mode only) ---
    if (isTileset) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SeparatorText("Auto-fill Tiles");

        ImGui::Checkbox("Generate tile grid##AutoFillGrid", &m_autoFillGrid);

        if (m_autoFillGrid) {
            if (beginInputTable("SlicerFillTable")) {
                beginInputRow("Fill Pattern", 14.0f * ImGui::GetFontSize());
                if (ImGui::RadioButton("Sequential##FillSeq", m_fillPattern == FillPattern::SEQUENTIAL)) {
                    m_fillPattern = FillPattern::SEQUENTIAL;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Single Rect##FillSingle", m_fillPattern == FillPattern::SINGLE_RECT)) {
                    m_fillPattern = FillPattern::SINGLE_RECT;
                }

                if (m_fillPattern == FillPattern::SINGLE_RECT) {
                    beginInputRow("Rect ID", 6.0f * ImGui::GetFontSize());
                    int maxRect = std::max(0, (int)m_previewRects.size() - 1);
                    ImGui::DragInt("##SingleRectId", &m_singleRectId, 0.1f, 0, maxRect);
                }

                beginInputRow("Grid Columns", 6.0f * ImGui::GetFontSize());
                ImGui::DragInt("##GridCols", &m_gridColumns, 0.1f, 1, 256);

                beginInputRow("Grid Rows", 6.0f * ImGui::GetFontSize());
                ImGui::DragInt("##GridRows", &m_gridRows, 0.1f, 1, 256);

                if (ImGui::Checkbox("Auto tile size from cell##AutoTileSize", &m_autoTileSize)) {
                    if (m_autoTileSize) {
                        m_previewDirty = true;
                    }
                }

                if (!m_autoTileSize) {
                    beginInputRow("Tile Width", 6.0f * ImGui::GetFontSize());
                    ImGui::DragFloat("##TileW", &m_tileWidth, 1.0f, 1.0f, 4096.0f, "%.0f");

                    beginInputRow("Tile Height", 6.0f * ImGui::GetFontSize());
                    ImGui::DragFloat("##TileH", &m_tileHeight, 1.0f, 1.0f, 4096.0f, "%.0f");
                }

                ImGui::EndTable();
            }

            int totalTiles = m_gridColumns * m_gridRows;
            ImGui::Text("Will generate %d tiles (%d x %d)", totalTiles, m_gridColumns, m_gridRows);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons
    float windowWidth = ImGui::GetWindowSize().x;
    float buttonsWidth = 260;
    ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

    ImGui::BeginDisabled(m_previewRects.empty());
    if (ImGui::Button("Apply", ImVec2(120, 0))) {
        if (m_onApply) {
            m_onApply(buildResult());
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
