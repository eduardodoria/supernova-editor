// ProjectSaveDialog.cpp
#include "ProjectSaveDialog.h"
#include "Util.h"
#include "Backend.h"
#include "external/IconsFontAwesome6.h"

namespace Supernova {
namespace Editor {

void ProjectSaveDialog::open(const std::string& defaultName,
        std::function<void(const std::string&, const fs::path&)> onSave,
        std::function<void()> onCancel) {
    m_isOpen = true;
    m_projectName = defaultName;
    m_projectPath.clear();
    m_onSave = onSave;
    m_onCancel = onCancel;

    // Set default project name
    strncpy(m_projectNameBuffer, defaultName.c_str(), sizeof(m_projectNameBuffer) - 1);
    m_projectNameBuffer[sizeof(m_projectNameBuffer) - 1] = '\0'; // Ensure null termination
}

void ProjectSaveDialog::show() {
    if (!m_isOpen) {
        return;
    }

    // Setup modal popup
    ImGui::OpenPopup("Save Project##SaveProjectModal");

    // Center the popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // Setup window flags for modal dialog
    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_Modal;

    bool popupOpen = ImGui::BeginPopupModal("Save Project##SaveProjectModal", nullptr, flags);

    if (popupOpen) {
        // Project name input
        ImGui::Text("Project Name:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##projectName", m_projectNameBuffer, sizeof(m_projectNameBuffer));

        // Project path display and button
        ImGui::Text("Path:");
        
        ImGui::SetNextItemWidth(300 - 80 - ImGui::GetStyle().ItemSpacing.x); // Width minus button width and spacing
        
        // Show path or placeholder if empty
        std::string pathDisplay = m_projectPath.empty() ? "<No path selected>" : m_projectPath.string();
        ImGui::InputText("##projectPath", const_cast<char*>(pathDisplay.c_str()), 
                        pathDisplay.size(), ImGuiInputTextFlags_ReadOnly);
        
        ImGui::SameLine();
        if (ImGui::Button("Browse", ImVec2(80, 0))) {
            // Get home directory as default path
            std::string homeDirPath;
            #ifdef _WIN32
            homeDirPath = std::filesystem::path(getenv("USERPROFILE")).string();
            #else
            homeDirPath = std::filesystem::path(getenv("HOME")).string();
            #endif
            
            // Get project name from buffer for the dialog
            std::string projectName = m_projectNameBuffer;
            if (projectName.empty()) {
                projectName = "NewProject";
            }
            
            // Open folder dialog
            std::string selectedPath = Util::saveFileDialog(homeDirPath, projectName, false);
            
            if (!selectedPath.empty()) {
                m_projectPath = selectedPath;
            }
        }

        // Validation
        std::string projectName = m_projectNameBuffer;
        bool canSave = !projectName.empty() && !m_projectPath.empty();
        
        // Check if the selected directory is empty if it exists
        bool directoryWarning = false;
        if (!m_projectPath.empty() && std::filesystem::exists(m_projectPath)) {
            if (!std::filesystem::is_empty(m_projectPath)) {
                directoryWarning = true;
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
                                  "Warning: Selected directory is not empty!");
            }
        }

        ImGui::Separator();

        // Center the buttons
        float windowWidth = ImGui::GetWindowSize().x;
        float buttonsWidth = 250; // Total width for both buttons and spacing
        ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

        // Buttons
        ImGui::BeginDisabled(!canSave);
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            if (m_onSave) {
                m_onSave(projectName, m_projectPath);
            }
            m_isOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_isOpen = false;
            if (m_onCancel) {
                m_onCancel();
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else {
        // If the popup isn't open anymore but our state says it should be,
        // update our state and make sure to call the cancel callback
        if (m_isOpen) {
            m_isOpen = false;
            if (m_onCancel) {
                m_onCancel();
            }
        }
    }
}

} // namespace Editor
} // namespace Supernova