#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    struct SceneProject;

    class Conector{
    private:
        void* libHandle;

        bool fileExists(const fs::path& path);
        void* loadSharedLibrary(const std::string& libPath);
        void unloadSharedLibrary(void* libHandle);

    public:
        Conector();
        virtual ~Conector();

        bool connect(fs::path projectPath, std::string libName);
        void disconnect();
        void execute(SceneProject* sceneProject);

        bool isLibraryConnected() const;
    };

}
