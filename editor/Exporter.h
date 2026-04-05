#pragma once

#include "Project.h"
#include "Generator.h"
#include "shader/ShaderBuilder.h"
#include "ShaderDataSerializer.h"

#include <filesystem>
#include <set>
#include <vector>
#include <string>
#include <functional>
#include <atomic>

namespace Supernova::Editor {

    namespace fs = std::filesystem;

    struct ExportConfig {
        fs::path targetDir;
        fs::path assetsDir;
        std::set<ShaderKey> selectedShaderKeys;
        std::set<Platform> selectedPlatforms;
    };

    struct ExportProgress {
        std::string currentStep;
        float overallProgress = 0.0f;
        bool finished = false;
        bool failed = false;
        std::string errorMessage;
    };

    class Exporter {
    private:
        Project* project;
        ExportConfig config;
        ExportProgress progress;

        ShaderBuilder shaderBuilder;

        void setProgress(const std::string& step, float value);
        void setError(const std::string& message);

        bool checkTargetDir();
        bool cleanGenerated();
        bool saveAllScenes();
        bool copyGenerated();
        bool copyAssets();
        bool copyCppScripts();
        bool copyEngine();
        bool buildAndSaveShaders();
        bool generateCMakeLists();

    public:
        Exporter();

        void startExport(Project* project, const ExportConfig& config);
        const ExportProgress& getProgress() const;

        static std::string getShaderDisplayName(ShaderType type, uint32_t properties);
        static std::string getPlatformName(Platform platform);
    };

}
