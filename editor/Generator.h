#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <filesystem>
#include <future>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include "Scene.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>  // pid_t
#include <spawn.h>
#endif

namespace fs = std::filesystem;

namespace Supernova::Editor {

    struct ScriptSource {
        fs::path path;
        fs::path headerPath;
        std::string className;
        Scene* scene;
        Entity entity;
    };

    struct SceneBuildInfo {
        uint32_t id;
        std::string name;
        bool isMain;
    };

    struct SceneStackInfo {
        std::string stackName;
        uint32_t mainSceneId;
        std::vector<uint32_t> sceneIds;
    };

    class Generator {
    private:
        std::future<void> buildFuture;
        std::atomic<bool> lastBuildSucceeded;
        std::atomic<bool> cancelRequested;

        static fs::path getExecutableDir();
        static fs::path getGeneratedPath(const fs::path& projectInternalPath);

    #ifdef _WIN32
        HANDLE currentProcessHandle;
        std::mutex processHandleMutex;
    #else
        pid_t currentProcessPid;
        std::mutex processPidMutex;
    #endif

        bool configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool runCommand(const std::string& command, const fs::path& workingDir);
        std::string getPlatformCMakeConfig();
        std::string getPlatformEditorHeader();
        std::string getPlatformEditorSource(const fs::path& projectPath);

        void writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<ScriptSource>& scriptFiles, const std::vector<SceneBuildInfo>& scenes);
        void terminateCurrentProcess();

    public:
        Generator();
        ~Generator();
        void writeSceneSource(Scene* scene, const std::string& sceneName, const std::vector<Entity>& entities, const Entity camera, const fs::path& projectPath, const fs::path& projectInternalPath);
        void clearSceneSource(const std::string& sceneName, const fs::path& projectInternalPath);
        void configure(const std::vector<SceneBuildInfo>& scenes, const std::vector<Editor::SceneStackInfo> stacks, std::string libName, const std::vector<ScriptSource>& scriptFiles, const fs::path& projectPath, const fs::path& projectInternalPath);
        void build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
        bool didLastBuildSucceed() const;
        // Request cancellation asynchronously. Returns a future you can wait on.
        std::future<void> cancelBuild();
    };
}

#endif