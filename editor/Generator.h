// Generator.h
#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <filesystem>
#include <future>
#include <vector>
#include "Scene.h"

namespace fs = std::filesystem;

namespace Supernova::Editor {

    struct ScriptSource {
        fs::path path;
        std::string className;
        Scene* scene;
        Entity entity;
    };

    class Generator {
    private:
        std::future<void> buildFuture;

        bool configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool writeIfChanged(const fs::path& filePath, const std::string& newContent);
        bool tryIncludeHeader(const fs::path& p, const fs::path& projectPath, std::unordered_set<std::string>& included, std::string& sourceContent);

        void writeSourceFiles(const fs::path& projectPath, const std::vector<ScriptSource>& scriptFiles);

    public:
        Generator();
        ~Generator();

        // New overload that accepts discovered script files
        void build(fs::path projectPath, const std::vector<ScriptSource>& scriptFiles);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
    };
}

#endif