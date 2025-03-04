#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "yaml-cpp/yaml.h"

namespace Supernova::Editor {

class AppSettings {
private:
    static std::filesystem::path configFilePath;
    static YAML::Node settingsData;
    
    // Recent projects list (max 10)
    static std::vector<std::filesystem::path> recentProjects;
    static std::filesystem::path lastProjectPath;
    
    // Window settings
    static int windowWidth;
    static int windowHeight;
    static bool isMaximized;
    
    // Private methods
    static void ensureConfigDirectory();
    
public:
    // Initialization
    static bool initialize();
    
    // Project settings
    static std::filesystem::path getLastProjectPath();
    static void setLastProjectPath(const std::filesystem::path& path);
    
    static std::vector<std::filesystem::path> getRecentProjects();
    static void addToRecentProjects(const std::filesystem::path& path, bool needSave = true);
    static void clearRecentProjects();
    
    // Window settings
    static int getWindowWidth();
    static int getWindowHeight();
    static bool getIsMaximized();
    
    static void setWindowWidth(int width);
    static void setWindowHeight(int height);
    static void setIsMaximized(bool maximized);
    
    // Load and save settings
    static bool loadSettings();
    static bool saveSettings();
};

} // namespace Supernova::Editor