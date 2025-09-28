// Generator.h
#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <filesystem>
#include <future>
#include <vector>

namespace fs = std::filesystem;

namespace Supernova::Editor {

    class Generator {
    private:
        std::future<void> buildFuture;

        bool configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType);
        bool writeIfChanged(const fs::path& filePath, const std::string& newContent);

        void writeSourceFiles(const fs::path& projectPath, const std::vector<fs::path>& scriptFiles);

    public:
        Generator();
        ~Generator();

        // New overload that accepts discovered script files
        void build(fs::path projectPath, const std::vector<fs::path>& scriptFiles);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
    };
}

#endif