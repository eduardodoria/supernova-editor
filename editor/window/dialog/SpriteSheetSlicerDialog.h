#pragma once

#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "util/SpriteFrameData.h"
#include "Engine.h"
#include "texture/Texture.h"

namespace Supernova {
namespace Editor {

class SpriteSheetSlicerDialog {
public:
    enum class SliceMode {
        GRID,       // by columns x rows
        CELL_SIZE   // by cell width x height
    };

    struct SliceResult {
        std::vector<SpriteFrameData> frames;
    };

private:
    bool m_isOpen = false;

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

    bool m_previewDirty = true;
    std::vector<SpriteFrameData> m_previewFrames;

    std::function<void(const SliceResult&)> m_onApply;
    std::function<void()> m_onCancel;

    void generatePreview();

public:
    SpriteSheetSlicerDialog() = default;
    ~SpriteSheetSlicerDialog() = default;

    void open(const Texture& previewTexture,
              int sheetWidth, int sheetHeight,
              std::function<void(const SliceResult&)> onApply,
              std::function<void()> onCancel = nullptr);

    void show();
    bool isOpen() const { return m_isOpen; }
    void close() { m_isOpen = false; }
};

} // namespace Editor
} // namespace Supernova
