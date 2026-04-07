#include "ProjectSettingsWindow.h"
#include "util/FileDialogs.h"
#include "Backend.h"
#include "window/Widgets.h"
#include "external/IconsFontAwesome6.h"

namespace Supernova::Editor {

static const char* scalingModeNames[] = { "Fit Width", "Fit Height", "Letterbox", "Crop", "Stretch", "Native" };
static const Scaling scalingModeValues[] = { Scaling::FITWIDTH, Scaling::FITHEIGHT, Scaling::LETTERBOX, Scaling::CROP, Scaling::STRETCH, Scaling::NATIVE };
static const int scalingModeCount = sizeof(scalingModeValues) / sizeof(scalingModeValues[0]);

static const char* textureStrategyNames[] = { "Fit", "Resize", "None" };
static const TextureStrategy textureStrategyValues[] = { TextureStrategy::FIT, TextureStrategy::RESIZE, TextureStrategy::NONE };
static const int textureStrategyCount = sizeof(textureStrategyValues) / sizeof(textureStrategyValues[0]);

static int findScalingIndex(Scaling mode) {
    for (int i = 0; i < scalingModeCount; i++) {
        if (scalingModeValues[i] == mode) return i;
    }
    return 0;
}

static int findTextureStrategyIndex(TextureStrategy strategy) {
    for (int i = 0; i < textureStrategyCount; i++) {
        if (textureStrategyValues[i] == strategy) return i;
    }
    return 0;
}

void ProjectSettingsWindow::open(Project* project) {
    m_isOpen = true;
    m_project = project;
    m_canvasWidth = project->getWindowWidth();
    m_canvasHeight = project->getWindowHeight();
    m_scalingModeIndex = findScalingIndex(project->getScalingMode());
    m_textureStrategyIndex = findTextureStrategyIndex(project->getTextureStrategy());
    m_assetsDir = project->getAssetsDir();
    m_luaDir = project->getLuaDir();
}

void ProjectSettingsWindow::show() {
    if (!m_isOpen) return;

    ImGui::OpenPopup("Project Settings##ProjectSettingsModal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(500, 0),
        ImVec2(500, ImGui::GetMainViewport()->WorkSize.y * 0.9f)
    );

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_Modal |
                             ImGuiWindowFlags_AlwaysAutoResize;

    bool popupOpen = ImGui::BeginPopupModal("Project Settings##ProjectSettingsModal", &m_isOpen, flags);

    if (popupOpen) {
        if (!m_isOpen) {
            ImGui::CloseCurrentPopup();
        } else {
            drawSettings();
        }
        ImGui::EndPopup();
    }
}

void ProjectSettingsWindow::drawSettings() {
    ImGui::PushItemWidth(-1);
    ImGui::BeginTable("project_settings", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 120);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

    // Canvas width row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Canvas Width");
    ImGui::TableNextColumn();
    {
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##CanvasWidth", &m_canvasWidth);
        if (m_canvasWidth < 1) m_canvasWidth = 1;
    }

    // Canvas height row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Canvas Height");
    ImGui::TableNextColumn();
    {
        ImGui::SetNextItemWidth(-1);
        ImGui::InputInt("##CanvasHeight", &m_canvasHeight);
        if (m_canvasHeight < 1) m_canvasHeight = 1;
    }

    // Scaling mode row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Scaling Mode");
    ImGui::TableNextColumn();
    {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##ScalingMode", scalingModeNames[m_scalingModeIndex])) {
            for (int i = 0; i < scalingModeCount; i++) {
                bool isSelected = (m_scalingModeIndex == i);
                if (ImGui::Selectable(scalingModeNames[i], isSelected)) {
                    m_scalingModeIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    // Texture strategy row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Texture Strategy");
    ImGui::TableNextColumn();
    {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##TextureStrategy", textureStrategyNames[m_textureStrategyIndex])) {
            for (int i = 0; i < textureStrategyCount; i++) {
                bool isSelected = (m_textureStrategyIndex == i);
                if (ImGui::Selectable(textureStrategyNames[i], isSelected)) {
                    m_textureStrategyIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    // Assets directory row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Assets Directory");
    ImGui::TableNextColumn();
    {
        float browseWidth = ImGui::CalcTextSize("Browse").x + ImGui::GetStyle().FramePadding.x * 2;
        float inputWidth = ImGui::GetContentRegionAvail().x - browseWidth - ImGui::GetStyle().ItemSpacing.x;

        Vector2 pathSize = Vector2(inputWidth, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2);
        fs::path assetsDisplay = (m_assetsDir.empty() || m_assetsDir == ".") ? fs::path("<Project root>") : m_assetsDir;
        Widgets::pathDisplay("##AssetsPath", assetsDisplay, pathSize);

        ImGui::SameLine();
        if (ImGui::Button("Browse##assets")) {
            std::string defaultPath = m_project ? m_project->getProjectPath().string() : "";
            std::string selectedPath = FileDialogs::openFileDialog(defaultPath, FILE_DIALOG_ALL, true);
            if (!selectedPath.empty()) {
                std::error_code ec;
                fs::path relPath = fs::relative(fs::path(selectedPath), m_project->getProjectPath(), ec);
                m_assetsDir = (ec || relPath.empty()) ? fs::path(".") : relPath;
            }
        }
    }

    // Lua directory row
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("Lua Directory");
    ImGui::TableNextColumn();
    {
        float browseWidth = ImGui::CalcTextSize("Browse").x + ImGui::GetStyle().FramePadding.x * 2;
        float inputWidth = ImGui::GetContentRegionAvail().x - browseWidth - ImGui::GetStyle().ItemSpacing.x;

        Vector2 pathSize = Vector2(inputWidth, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2);
        fs::path luaDisplay = (m_luaDir.empty() || m_luaDir == ".") ? fs::path("<Project root>") : m_luaDir;
        Widgets::pathDisplay("##LuaPath", luaDisplay, pathSize);

        ImGui::SameLine();
        if (ImGui::Button("Browse##lua")) {
            std::string defaultPath = m_project ? m_project->getProjectPath().string() : "";
            std::string selectedPath = FileDialogs::openFileDialog(defaultPath, FILE_DIALOG_ALL, true);
            if (!selectedPath.empty()) {
                std::error_code ec;
                fs::path relPath = fs::relative(fs::path(selectedPath), m_project->getProjectPath(), ec);
                m_luaDir = (ec || relPath.empty()) ? fs::path(selectedPath) : relPath;
            }
        }
    }

    ImGui::EndTable();
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- Buttons ---
    float windowWidth = ImGui::GetWindowSize().x;
    float buttonsWidth = 250;
    ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

    if (ImGui::Button("OK", ImVec2(120, 0))) {
        m_project->setWindowSize(m_canvasWidth, m_canvasHeight);
        m_project->setScalingMode(scalingModeValues[m_scalingModeIndex]);
        m_project->setTextureStrategy(textureStrategyValues[m_textureStrategyIndex]);
        m_project->setAssetsDir(m_assetsDir);
        m_project->setLuaDir(m_luaDir);
        m_isOpen = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        m_isOpen = false;
        ImGui::CloseCurrentPopup();
    }
}

} // namespace Supernova::Editor
