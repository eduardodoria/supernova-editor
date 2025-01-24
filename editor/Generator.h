#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <unordered_map>
#include <vector>

namespace Supernova::Editor{

    class Generator{
    private:

        std::unordered_map<std::string, std::string> compilerCache;

        std::string executeCommand(const std::string& command);
        std::string getOperatingSystem();
        std::string getCompilerVersion(const std::string& compiler);
        std::string findCompiler();

    public:
        Generator();

        void buildSharedLibrary(
            const std::string& compiler,
            const std::vector<std::string>& sourceFiles,
            const std::string& outputFile,
            bool debug = false,
            const std::vector<std::string>& additionalFlags = {});

        void build();

    };

}

#endif /* GENERATOR_H */