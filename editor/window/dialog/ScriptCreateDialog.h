#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <algorithm>
#include <cctype>
#include "imgui.h"
#include "component/ScriptComponent.h"

namespace Supernova {
namespace Editor {

namespace fs = std::filesystem;

class ScriptCreateDialog {
private:
    bool m_isOpen = false;
    fs::path m_projectPath;
    std::string m_selectedPath;
    char m_baseNameBuffer[128] = "";
    ScriptType m_scriptType = ScriptType::SUBCLASS;

    // For C++ scripts, headerPath/sourcePath are valid; for Lua scripts, luaPath is valid.
    std::function<void(const fs::path& headerPath,
                       const fs::path& sourcePath,
                       const std::string& classOrModuleName,
                       ScriptType type)> m_onCreate;

    std::function<void()> m_onCancel;

    void displayDirectoryTree(const fs::path& rootPath, const fs::path& currentPath);

    std::string sanitizeClassName(const std::string& in) const;

    // New helper paths
    fs::path makeHeaderPath(const std::string& className) const;
    fs::path makeSourcePath(const std::string& className) const;
    fs::path makeLuaPath(const std::string& moduleName) const;

    // Unified writer (handles C++ and Lua)
    void writeFiles(const fs::path& headerPath,
                    const fs::path& sourcePath,
                    const std::string& classOrModuleName,
                    ScriptType type);

    void finalizeCreation(const fs::path& headerPath,
                          const fs::path& sourcePath,
                          const std::string& name);

public:
    ScriptCreateDialog() = default;
    ~ScriptCreateDialog() = default;

    void open(const fs::path& projectPath,
              const std::string& defaultBaseName,
              std::function<void(const fs::path& headerPath,
                                 const fs::path& sourcePath,
                                 const std::string& classOrModuleName,
                                 ScriptType type)> onCreate,
              std::function<void()> onCancel = nullptr);

    void show();
    bool isOpen() const { return m_isOpen; }
    void close() { m_isOpen = false; }
};

} // namespace Editor
} // namespace Supernova