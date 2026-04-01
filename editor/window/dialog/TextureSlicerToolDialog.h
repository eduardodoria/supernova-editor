#pragma once

#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "math/Rect.h"
#include "math/Vector2.h"
#include "Engine.h"
#include "texture/Texture.h"

namespace Supernova {
namespace Editor {

class TextureSlicerToolDialog {
public:
    enum class Mode {
        SPRITE,
        TILESET
    };

    enum class SliceMode {
        GRID,       // by columns x rows
        CELL_SIZE   // by cell width x height
    };

    enum class FillPattern {
        SEQUENTIAL,
        SINGLE_RECT
    };

    struct SlicedRect {
        std::string name;
        Rect rect;
    };

    struct TileGridEntry {
        std::string name;
        int rectId;
        Vector2 position;
        float width;
        float height;
    };

    struct SliceResult {
        std::vector<SlicedRect> rects;
        bool hasTileGrid = false;
        std::vector<TileGridEntry> tiles;
    };

private:
    bool m_isOpen = false;
    Mode m_mode = Mode::SPRITE;

    SliceMode m_sliceMode = SliceMode::GRID;

    int m_columns = 4;
    int m_rows = 4;
    int m_cellWidth = 32;
    int m_cellHeight = 32;
    int m_offsetX = 0;
    int m_offsetY = 0;
    int m_paddingX = 0;
    int m_paddingY = 0;

    int m_sheetWidth = 0;
    int m_sheetHeight = 0;

    Texture m_previewTexture;

    char m_prefixBuffer[64] = "frame_";

    // Tile grid options (only used in TILESET mode)
    bool m_autoFillGrid = false;
    FillPattern m_fillPattern = FillPattern::SEQUENTIAL;
    int m_singleRectId = 0;
    int m_gridColumns = 10;
    int m_gridRows = 10;
    float m_tileWidth = 32.0f;
    float m_tileHeight = 32.0f;
    bool m_autoTileSize = true;

    bool m_previewDirty = true;
    std::vector<SlicedRect> m_previewRects;

    std::function<void(const SliceResult&)> m_onApply;
    std::function<void()> m_onCancel;

    void generatePreview();
    SliceResult buildResult();

public:
    TextureSlicerToolDialog() = default;
    ~TextureSlicerToolDialog() = default;

    void open(const Texture& previewTexture,
              int sheetWidth, int sheetHeight,
              std::function<void(const SliceResult&)> onApply,
              std::function<void()> onCancel = nullptr);

    void openTileset(const Texture& previewTexture,
                     int sheetWidth, int sheetHeight,
                     unsigned int tilemapWidth, unsigned int tilemapHeight,
                     std::function<void(const SliceResult&)> onApply,
                     std::function<void()> onCancel = nullptr);

    void show();
    bool isOpen() const { return m_isOpen; }
    void close() { m_isOpen = false; }
};

} // namespace Editor
} // namespace Supernova
