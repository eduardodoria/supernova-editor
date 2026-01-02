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

    struct SceneData {
        Scene* scene;
        std::string name;
        std::vector<Entity> entities;
    };

    class Generator {
    private:
        std::future<void> buildFuture;
        std::atomic<bool> lastBuildSucceeded;
        std::atomic<bool> cancelRequested;

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
        bool writeIfChanged(const fs::path& filePath, const std::string& newContent);

        void writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<ScriptSource>& scriptFiles);
        void terminateCurrentProcess();

    public:
        Generator();
        ~Generator();

        void configure(const std::vector<SceneData>& scenes, const fs::path& projectInternalPath);
        void build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath, std::string libName, const std::vector<ScriptSource>& scriptFiles);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
        bool didLastBuildSucceed() const;
        // Request cancellation asynchronously. Returns a future you can wait on.
        std::future<void> cancelBuild();
    };
}

#endif