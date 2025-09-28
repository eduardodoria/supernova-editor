#ifndef CONECTOR_H
#define CONECTOR_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    struct SceneProject;

    class Conector{
    private:
        void* libHandle;
        std::string libName;

        bool fileExists(const fs::path& path);
        void* loadSharedLibrary(const std::string& libPath);
        void unloadSharedLibrary(void* libHandle);

    public:
        Conector();
        virtual ~Conector();

        bool connect(fs::path projectPath);
        void execute(SceneProject* sceneProject);
    };

}

#endif /* CONECTOR_H */