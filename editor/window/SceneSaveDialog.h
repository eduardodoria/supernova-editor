// SceneSaveDialog.h
#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include "imgui.h"

namespace Supernova {
namespace Editor {

namespace fs = std::filesystem;

class SceneSaveDialog {
private:
    void displayDirectoryTree(const fs::path& rootPath, const fs::path& currentPath);

    bool m_isOpen = false;
    fs::path m_projectPath;
    std::string m_selectedPath;
    char m_fileNameBuffer[256] = "";
    bool m_exitAfterSave = false;

    std::function<void(const fs::path&)> m_onSave;

public:
    SceneSaveDialog() = default;
    ~SceneSaveDialog() = default;

    void open(const fs::path& projectPath, const std::string& defaultName, 
              std::function<void(const fs::path&)> onSave);
    void show();
    bool isOpen() const { return m_isOpen; }
    void close() { m_isOpen = false; }
    void setExitAfterSave(bool exitAfterSave) { m_exitAfterSave = exitAfterSave; }
};

} // namespace Editor
} // namespace Supernova