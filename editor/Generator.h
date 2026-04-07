#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <filesystem>
#include <future>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_set>
#include <map>
#include "Scene.h"
#include "Factory.h"
#include "util/EntityBundle.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>  // pid_t
#include <spawn.h>
#endif

namespace fs = std::filesystem;

namespace Supernova::Editor {

    struct ScriptPropertyInfo {
        std::string name;
        bool isPtr;
        std::string ptrTypeName;
    };

    struct SceneScriptSource {
        fs::path path;
        fs::path headerPath;
        std::string className;
        std::vector<ScriptPropertyInfo> properties;
    };


    struct SceneBuildInfo {
        uint32_t id;
        std::string name;
        std::vector<uint32_t> involvedScenes; // IDs of all scenes involved in this stack (main + layers)
        bool isMain;
    };

    struct BundleSceneInfo {
        std::filesystem::path bundlePath; // relative path of the .bundle file
        std::string functionName;         // generated C++ function name (e.g. create_bundle_X)
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
        void clearStaleCMakeCache(const fs::path& projectPath, const fs::path& buildPath);
        std::string getPlatformCMakeConfig();
        std::string getPlatformEditorHeader();
        std::string getPlatformEditorSource(const fs::path& projectPath);
        std::string buildInitSceneScriptsSource(const std::vector<SceneScriptSource>& scriptFiles);
        std::string buildCleanupSceneScriptsSource(const std::vector<SceneScriptSource>& scriptFiles);

        void writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<SceneScriptSource>& scriptFiles, const std::vector<SceneBuildInfo>& scenes, const std::vector<BundleSceneInfo>& bundles);
        void terminateCurrentProcess();

    public:
        Generator();
        ~Generator();
        std::vector<BundleInstanceInfo> writeBundleSources(const std::map<fs::path, EntityBundle>& entityBundles, uint32_t sceneId, const fs::path& projectPath, const fs::path& projectInternalPath);
        void writeSceneSource(Scene* scene, const std::string& sceneName, const std::vector<Entity>& entities, const Entity camera, const fs::path& projectPath, const fs::path& projectInternalPath, std::vector<BundleInstanceInfo>& bundleInstances);
        void clearSceneSource(const std::string& sceneName, const fs::path& projectInternalPath);
        void configure(const std::vector<SceneBuildInfo>& scenes, std::string libName, const std::vector<SceneScriptSource>& scriptFiles, const std::vector<BundleSceneInfo>& bundles, const fs::path& projectPath, const fs::path& projectInternalPath, Scaling scalingMode = Scaling::FITWIDTH, TextureStrategy textureStrategy = TextureStrategy::RESIZE, unsigned int canvasWidth = 1280, unsigned int canvasHeight = 720);
        void build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
        bool didLastBuildSucceed() const;
        // Request cancellation asynchronously. Returns a future you can wait on.
        std::future<void> cancelBuild();
    };
}

#endif