#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Supernova {
    class Scene;
}

namespace Supernova::Editor{

    class Conector{
    private:
        void* libHandle;

        bool fileExists(const fs::path& path);
        void* loadSharedLibrary(const std::string& libPath);
        void unloadSharedLibrary(void* libHandle);

    public:
        Conector();
        virtual ~Conector();

        bool connect(const fs::path& buildPath, std::string libName);
        void disconnect();
        void cleanup(Supernova::Scene* scene);
        void execute(Supernova::Scene* scene);

        bool isLibraryConnected() const;
    };

}