#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <future>

namespace fs = std::filesystem;

namespace Supernova::Editor {

    class Generator {
    private:
        std::string compiler;

        std::unordered_map<std::string, std::string> compilerCache;
        std::future<void> buildFuture;

        std::string executeCommand(const std::string& command);
        std::string getOperatingSystem();
        std::string getCompilerVersion(const std::string& compiler);
        std::string findCompiler();

    public:
        Generator();
        ~Generator();

        void setup();

        void buildSharedLibrary(
            const std::string& compiler,
            const std::vector<std::string>& sourceFiles,
            const std::string& outputFile,
            const std::vector<std::string>& includeDirs = {},
            bool debug = false,
            const fs::path& libPath = "",
            const std::vector<std::string>& libraries = {},
            const std::vector<std::string>& additionalFlags = {});

        void build(fs::path projectPath);
        bool isBuildInProgress() const;
        void waitForBuildToComplete();
    };

}

#endif /* GENERATOR_H */