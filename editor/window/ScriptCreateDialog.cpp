#include "ScriptCreateDialog.h"
#include "external/IconsFontAwesome6.h"
#include <fstream>

namespace Supernova {
namespace Editor {

void ScriptCreateDialog::open(const fs::path& projectPath, const std::string& defaultBaseName,
                              std::function<void(const fs::path& headerPath, const fs::path& sourcePath, const std::string& className)> onCreate,
                              std::function<void()> onCancel) {
    m_isOpen = true;
    m_projectPath = projectPath;
    m_selectedPath = projectPath.string();
    m_onCreate = onCreate;
    m_onCancel = onCancel;

    std::string base = defaultBaseName.empty() ? "NewScript" : defaultBaseName;
    strncpy(m_baseNameBuffer, base.c_str(), sizeof(m_baseNameBuffer) - 1);
    m_baseNameBuffer[sizeof(m_baseNameBuffer) - 1] = '\0';
}

std::string ScriptCreateDialog::sanitizeClassName(const std::string& in) const {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
            out.push_back(c);
    }
    if (out.empty()) out = "NewScript";
    if (std::isdigit(static_cast<unsigned char>(out[0])))
        out = "_" + out;
    return out;
}

void ScriptCreateDialog::writeFiles(const fs::path& headerPath, const fs::path& sourcePath, const std::string& className) {
    fs::create_directories(headerPath.parent_path());

    // Header
    {
        std::ofstream h(headerPath, std::ios::trunc);
        if (h) {
            h << "#pragma once\n\n";
            h << "#include \"Shape.h\"\n\n";
            h << "class " << className << " : public Supernova::Shape {\n";
            h << "public:\n";
            h << "    " << className << "(Supernova::Scene* scene, Supernova::Entity entity);\n";
            h << "    virtual ~" << className << "() = default;\n\n";
            h << "};\n";
        }
    }

    // Source
    {
        std::ofstream c(sourcePath, std::ios::trunc);
        if (c) {
            c << "#include \"" << headerPath.filename().string() << "\"\n\n";
            c << "using namespace Supernova;\n\n";
            c << className << "::" << className << "(Scene* scene, Entity entity): Shape(scene, entity) {\n";
            c << "    printf(\"" << className << " created!\\n\");\n";
            c << "}\n\n";
        }
    }
}

void ScriptCreateDialog::show() {
    if (!m_isOpen) return;

    ImGui::OpenPopup("Create Script##CreateScriptModal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_Modal;

    bool popupOpen = ImGui::BeginPopupModal("Create Script##CreateScriptModal", nullptr, flags);
    if (!popupOpen) {
        if (m_isOpen) {
            m_isOpen = false;
            if (m_onCancel) m_onCancel();
        }
        return;
    }

    if (ImGui::BeginChild("DirBrowser", ImVec2(300, 200), true)) {
        if (ImGui::BeginTable("DirTree", 1, ImGuiTableFlags_Resizable)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            bool rootOpen = true;
            ImGui::SetNextItemOpen(rootOpen, ImGuiCond_Always);
            bool isRootSelected = (m_selectedPath == m_projectPath.string());

            if (ImGui::TreeNodeEx("##root",
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth |
                (isRootSelected ? ImGuiTreeNodeFlags_Selected : 0))) {

                ImGui::SameLine(0, 0);
                ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "%s", ICON_FA_FOLDER_OPEN);
                ImGui::SameLine();
                ImGui::Text("Project Root");
                if (ImGui::IsItemClicked() ||
                    (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))) {
                    m_selectedPath = m_projectPath.string();
                }

                displayDirectoryTree(m_projectPath, m_projectPath);
                ImGui::TreePop();
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();

    ImGui::Text("Class / Base Name:");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##basename", m_baseNameBuffer, sizeof(m_baseNameBuffer));

    std::string baseName = m_baseNameBuffer;
    bool hasBase = !baseName.empty();

    std::string className = sanitizeClassName(baseName);
    fs::path headerPath = fs::path(m_selectedPath) / (className + ".h");
    fs::path sourcePath = fs::path(m_selectedPath) / (className + ".cpp");

    ImGui::TextWrapped("Will create:\n  %s\n  %s",
        fs::relative(headerPath, m_projectPath).string().c_str(),
        fs::relative(sourcePath, m_projectPath).string().c_str());

    bool headerExists = fs::exists(headerPath);
    bool sourceExists = fs::exists(sourcePath);
    if (headerExists || sourceExists) {
        ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
            "Warning: Existing file(s) will be overwritten.");
    }

    ImGui::Separator();

    float windowWidth = ImGui::GetWindowSize().x;
    float buttonsWidth = 250;
    ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

    ImGui::BeginDisabled(!hasBase);
    if (ImGui::Button("Create", ImVec2(120, 0))) {
        writeFiles(headerPath, sourcePath, className);
        if (m_onCreate) {
            m_onCreate(headerPath, sourcePath, className);
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

void ScriptCreateDialog::displayDirectoryTree(const fs::path& rootPath, const fs::path& currentPath) {
    try {
        std::vector<fs::path> subDirs;
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            if (entry.is_directory()) {
                subDirs.push_back(entry.path());
            }
        }
        std::sort(subDirs.begin(), subDirs.end());

        for (const auto& dirPath : subDirs) {
            std::string fname = dirPath.filename().string();
            if (!fname.empty() && fname[0] == '.')
                continue;

            ImGui::PushID(dirPath.string().c_str());
            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
            bool isSelected = (m_selectedPath == dirPath.string());
            if (isSelected)
                nodeFlags |= ImGuiTreeNodeFlags_Selected;

            bool hasSub = false;
            try {
                for (const auto& subEntry : fs::directory_iterator(dirPath)) {
                    if (subEntry.is_directory()) { hasSub = true; break; }
                }
            } catch (...) {}

            if (!hasSub) nodeFlags |= ImGuiTreeNodeFlags_Leaf;

            bool open = ImGui::TreeNodeEx("##dir", nodeFlags);
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(1.f, 0.8f, 0.f, 1.f), "%s", open ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER);
            ImGui::SameLine();
            ImGui::Text("%s", fname.c_str());

            if (ImGui::IsItemClicked() ||
                (ImGui::IsMouseClicked(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))) {
                m_selectedPath = dirPath.string();
            }

            if (open) {
                if (hasSub) {
                    displayDirectoryTree(rootPath, dirPath);
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    } catch (...) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error reading directory");
    }
}

} // namespace Editor
} // namespace Supernova