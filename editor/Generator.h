// Generator.h
#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <filesystem>
#include <future>

namespace fs = std::filesystem;

namespace Supernova::Editor {

    class Generator {
    private:
        std::future<void> buildFuture;

        bool configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool writeIfChanged(const fs::path& filePath, const std::string& newContent);

        void writeSourceFiles(const fs::path& projectPath);

    public:
        Generator();
        ~Generator();

        void build(fs::path projectPath);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
    };
}

#endif