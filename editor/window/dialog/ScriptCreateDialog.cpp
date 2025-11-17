#include "ScriptCreateDialog.h"
#include "external/IconsFontAwesome6.h"
#include <fstream>

namespace Supernova {
namespace Editor {

void ScriptCreateDialog::open(const fs::path& projectPath,
                              const std::string& defaultBaseName,
                              bool hasSubclass,
    std::function<void(const fs::path& headerPath,
                       const fs::path& sourcePath,
                       const fs::path& luaPath,
                       const std::string& classOrModuleName,
                       ScriptType type)> onCreate,
    std::function<void()> onCancel) {
    m_isOpen = true;
    m_projectPath = projectPath;
    m_selectedPath = projectPath.string();
    m_hasSubclass = hasSubclass;
    // If subclass exists, default to SCRIPT_CLASS (C++)
    m_scriptType = hasSubclass ? ScriptType::SCRIPT_CLASS : ScriptType::SUBCLASS;
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

fs::path ScriptCreateDialog::makeHeaderPath(const std::string& className) const {
    return fs::path(m_selectedPath) / (className + ".h");
}

fs::path ScriptCreateDialog::makeSourcePath(const std::string& className) const {
    return fs::path(m_selectedPath) / (className + ".cpp");
}

fs::path ScriptCreateDialog::makeLuaPath(const std::string& moduleName) const {
    return fs::path(m_selectedPath) / (moduleName + ".lua");
}

void ScriptCreateDialog::writeFiles(const fs::path& headerPath,
                                    const fs::path& sourcePath,
                                    const fs::path& luaPath,
                                    const std::string& classOrModuleName,
                                    ScriptType type) {
    bool isCppSubclass   = (type == ScriptType::SUBCLASS);
    bool isCppScriptBase = (type == ScriptType::SCRIPT_CLASS);
    bool isLua           = (type == ScriptType::SCRIPT_LUA);

    // C++ generation
    if (isCppSubclass || isCppScriptBase) {
        fs::create_directories(headerPath.parent_path());

        if (isCppSubclass) {
            // Header
            {
                std::ofstream h(headerPath, std::ios::trunc);
                if (h) {
                    h << "#pragma once\n\n";
                    h << "#include \"Shape.h\"\n";
                    h << "#include \"Engine.h\"\n";
                    h << "#include \"ScriptProperty.h\"\n\n";
                    h << "class " << classOrModuleName << " : public Supernova::Shape {\n";
                    h << "public:\n";
                    h << "    // Example properties - you can add more!\n";
                    h << "    SPROPERTY(\"Speed\")\n";
                    h << "    float speed = 5.0f;\n\n";
                    h << "    SPROPERTY(\"Is Active\")\n";
                    h << "    bool isActive = true;\n\n";
                    h << "    SPROPERTY(\"Target Position\")\n";
                    h << "    Supernova::Vector3 targetPosition = Supernova::Vector3(0, 0, 0);\n\n";
                    h << "    SPROPERTY(\"Mesh Color\", Color4)\n";
                    h << "    Supernova::Vector4 meshColor = Supernova::Vector4(1, 1, 1, 1);\n\n";
                    h << "    " << classOrModuleName << "(Supernova::Scene* scene, Supernova::Entity entity);\n";
                    h << "    virtual ~" << classOrModuleName << "();\n\n";
                    h << "    void onUpdate();\n";
                    h << "};\n";
                }
            }

            // Source
            {
                std::ofstream c(sourcePath, std::ios::trunc);
                if (c) {
                    c << "#include \"" << headerPath.filename().string() << "\"\n\n";
                    c << "using namespace Supernova;\n\n";
                    c << classOrModuleName << "::" << classOrModuleName << "(Scene* scene, Entity entity): Shape(scene, entity) {\n";
                    c << "    REGISTER_EVENT(onUpdate);\n\n";
                    c << "}\n\n";
                    c << classOrModuleName << "::~" << classOrModuleName << "() {\n";
                    c << "\n";
                    c << "}\n\n";
                    c << "void " << classOrModuleName << "::onUpdate() {\n";
                    c << "    if (!isActive) return;\n\n";
                    c << "    // Example: Move towards target position at 'speed' units per second\n";
                    c << "    float deltaTime = Engine::getDeltatime();\n";
                    c << "    Vector3 currentPos = getPosition();\n";
                    c << "    Vector3 direction = (targetPosition - currentPos).normalize();\n";
                    c << "    setPosition(currentPos + direction * speed * deltaTime);\n";
                    c << "    setColor(meshColor);\n";
                    c << "}\n\n";
                }
            }
        } else if (isCppScriptBase) {
            // Script Class
            // Header
            {
                std::ofstream h(headerPath, std::ios::trunc);
                if (h) {
                    h << "#pragma once\n\n";
                    h << "#include \"ScriptBase.h\"\n";
                    h << "#include \"Engine.h\"\n";
                    h << "#include \"ScriptProperty.h\"\n\n";
                    h << "class " << classOrModuleName << " : public Supernova::ScriptBase {\n";
                    h << "public:\n";
                    h << "    // Example properties - you can add more!\n";
                    h << "    SPROPERTY(\"Speed\")\n";
                    h << "    float speed = 5.0f;\n\n";
                    h << "    SPROPERTY(\"Is Active\")\n";
                    h << "    bool isActive = true;\n\n";
                    h << "    SPROPERTY(\"Counter\")\n";
                    h << "    int counter = 0;\n\n";
                    h << "    " << classOrModuleName << "(Supernova::Scene* scene, Supernova::Entity entity);\n";
                    h << "    ~" << classOrModuleName << "();\n\n";
                    h << "    void onUpdate();\n";
                    h << "};\n";
                }
            }

            // Source
            {
                std::ofstream c(sourcePath, std::ios::trunc);
                if (c) {
                    c << "#include \"" << headerPath.filename().string() << "\"\n\n";
                    c << "using namespace Supernova;\n\n";
                    c << classOrModuleName << "::" << classOrModuleName << "(Scene* scene, Entity entity): ScriptBase(scene, entity) {\n";
                    c << "    REGISTER_EVENT(onUpdate);\n\n";
                    c << "}\n\n";
                    c << classOrModuleName << "::~" << classOrModuleName << "() {\n";
                    c << "\n";
                    c << "}\n\n";
                    c << "void " << classOrModuleName << "::onUpdate() {\n";
                    c << "    if (!isActive) return;\n\n";
                    c << "    // Example: Increment counter every frame\n";
                    c << "    counter++;\n";
                    c << "    if (counter % 60 == 0) {\n";
                    c << "        printf(\"Counter: %d\\n\", counter);\n";
                    c << "    }\n";
                    c << "}\n\n";
                }
            }
        }
    }

    // Lua generation
    if (isLua) {
        fs::create_directories(luaPath.parent_path());
        std::ofstream f(luaPath, std::ios::trunc);
        if (f) {
            f << "-- " << classOrModuleName << ".lua\n";
            f << "-- Auto-generated by Supernova Editor\n\n";

            // Script table used as the instance prototype
            f << "local " << classOrModuleName << " = {\n";
            f << "    -- Editor-exposed properties\n";
            f << "    properties = {\n";
            f << "        {\n";
            f << "            name = \"speed\",\n";
            f << "            displayName = \"Speed\",\n";
            f << "            type = \"float\",\n";
            f << "            default = 5.0\n";
            f << "        },\n";
            f << "        {\n";
            f << "            name = \"isActive\",\n";
            f << "            displayName = \"Is Active\",\n";
            f << "            type = \"bool\",\n";
            f << "            default = true\n";
            f << "        }\n";
            f << "    }\n";
            f << "}\n\n";

            // init(self) – called once by the engine (Project::initializeLuaScripts)
            f << "function " << classOrModuleName << ":init()\n";
            f << "    -- 'self.scene' and 'self.entity' are provided by the engine\n";
            f << "    RegisterEvent(self, Engine.onUpdate, \"onUpdate\")\n";
            f << "    self.counter = 0\n";
            f << "end\n\n";

            // onUpdate(self) – optional; engine may call this every frame
            f << "function " << classOrModuleName << ":onUpdate()\n";
            f << "    if not self.isActive then return end\n\n";
            f << "    self.counter = self.counter + 1\n";
            f << "    if self.counter % 60 == 0 then\n";
            f << "        print(\"[\" .. tostring(self.entity) .. \"] Counter: \" .. self.counter)\n";
            f << "    end\n";
            f << "end\n\n";

            f << "return " << classOrModuleName << "\n";
        }
    }
}

void ScriptCreateDialog::show() {
    if (!m_isOpen) return;

    ImGui::OpenPopup("Create Script##CreateScriptModal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
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

    // Script Type Selection
    ImGui::Text("Script Type:");

    // C++ Subclass (only one per entity)
    ImGui::BeginDisabled(m_hasSubclass);
    if (ImGui::RadioButton("C++ Subclass", (int*)&m_scriptType, (int)ScriptType::SUBCLASS)) {}
    ImGui::EndDisabled();
    if (m_hasSubclass && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("Only one Subclass script is allowed per entity");
    }

    if (ImGui::RadioButton("C++ Script Class", (int*)&m_scriptType, (int)ScriptType::SCRIPT_CLASS)) {}

    if (ImGui::RadioButton("Lua Script", (int*)&m_scriptType, (int)ScriptType::SCRIPT_LUA)) {}

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

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

    std::string name = sanitizeClassName(baseName);

    // Compute possible paths
    fs::path headerPath;
    fs::path sourcePath;
    fs::path luaPath;

    bool isCpp = (m_scriptType == ScriptType::SUBCLASS ||
                m_scriptType == ScriptType::SCRIPT_CLASS);
    bool isLua = (m_scriptType == ScriptType::SCRIPT_LUA);

    if (isCpp) {
        headerPath = makeHeaderPath(name);
        sourcePath = makeSourcePath(name);
    }

    if (isLua) {
        luaPath = makeLuaPath(name);
    }

    // Display
    if (isCpp && isLua) {
        // (You probably won’t have a type that is both; kept for completeness)
    } else if (isCpp) {
        ImGui::TextWrapped("Will create:\n  %s\n  %s",
            fs::relative(headerPath, m_projectPath).string().c_str(),
            fs::relative(sourcePath, m_projectPath).string().c_str());

        bool headerExists = fs::exists(headerPath);
        bool sourceExists = fs::exists(sourcePath);
        if (headerExists || sourceExists) {
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
                "Warning: Existing file(s) will be overwritten.");
        }
    } else if (isLua) {
        ImGui::TextWrapped("Will create:\n  %s",
            fs::relative(luaPath, m_projectPath).string().c_str());

        bool luaExists = fs::exists(luaPath);
        if (luaExists) {
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f),
                "Warning: Existing file will be overwritten.");
        }
    }

    ImGui::Separator();

    float windowWidth = ImGui::GetWindowSize().x;
    float buttonsWidth = 250;
    ImGui::SetCursorPosX((windowWidth - buttonsWidth) * 0.5f);

    ImGui::BeginDisabled(!hasBase);
    if (ImGui::Button("Create", ImVec2(120, 0))) {
        fs::path useHeader, useSource, useLua;

        if (isCpp) {
            useHeader = headerPath;
            useSource = sourcePath;
        }
        if (isLua) {
            useLua = luaPath;
        }

        writeFiles(useHeader, useSource, useLua, name, m_scriptType);

        if (m_onCreate) {
            m_onCreate(useHeader, useSource, useLua, name, m_scriptType);
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