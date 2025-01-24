#ifndef CONECTOR_H
#define CONECTOR_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class Conector{
    private:
        void* libHandle;
        std::string libFile;

        bool fileExists(const std::string& path);
        void* loadSharedLibrary(const std::string& libPath);
        void unloadSharedLibrary(void* libHandle);

    public:
        Conector();
        virtual ~Conector();

        bool connect();
        void execute();
    };

}

#endif /* CONECTOR_H */