#include "Conector.h"

#include "editor/Out.h"
#include "Scene.h"

#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace Supernova;

Editor::Conector::Conector(){
    libHandle = nullptr;
    libName = "project_lib";
}

Editor::Conector::~Conector(){
    unloadSharedLibrary(libHandle);
}

bool Editor::Conector::fileExists(const fs::path& path) {
    return std::filesystem::exists(path);
}

void* Editor::Conector::loadSharedLibrary(const std::string& libPath) {
    #ifdef _WIN32
        HMODULE libHandle = LoadLibrary(libPath.c_str());
        if (!libHandle) {
            std::cerr << "Failed to load library: " << libPath << " (Error code: " << GetLastError() << ")\n";
            return nullptr;
        }
        std::cout << "Library loaded successfully: " << libPath << "\n";
        return libHandle;
    #else
        void* libHandle = dlopen(libPath.c_str(), RTLD_LAZY);
        if (!libHandle) {
            std::cerr << "Failed to load library: " << libPath << " (Error: " << dlerror() << ")\n";
            return nullptr;
        }
        std::cout << "Library loaded successfully: " << libPath << "\n";
        return libHandle;
    #endif
}

void Editor::Conector::unloadSharedLibrary(void* libHandle) {
    #ifdef _WIN32
        if (libHandle) {
            FreeLibrary(static_cast<HMODULE>(libHandle));
            std::cout << "Library unloaded successfully.\n";
        }
    #else
        if (libHandle) {
            dlclose(libHandle);
            std::cout << "Library unloaded successfully.\n";
        }
    #endif
    }

bool Editor::Conector::connect(fs::path projectPath){
    unloadSharedLibrary(libHandle);

    fs::path buildPath = projectPath / "build";

    std::string libPath = libName;
    #ifdef _WIN32
        libPath = libPath + ".dll";
    #else
        libPath = libPath + ".so";
    #endif

    Out::info("Checking for library file: %s", libPath.c_str());

    fs::path fullLibPath = buildPath / fs::path(libPath);
    if (fileExists(fullLibPath)) {
        std::cout << "Library file found!\n";
        libHandle = loadSharedLibrary(fullLibPath.string());
        if (libHandle) {
            return true;
        }
    } else {
        Out::error("Library file not found: %s", libPath.c_str());
    }

    return false;
}

void Editor::Conector::execute(){
    // Dynamically load and call the `sayHello` function
    using SayHelloFunc = void (*)(Scene*);
    #ifdef _WIN32
        SayHelloFunc sayHello = reinterpret_cast<SayHelloFunc>(GetProcAddress(static_cast<HMODULE>(libHandle), "sayHello"));
        if (!sayHello) {
            Out::error("Failed to find function 'sayHello' in the library (Error code: %i)", GetLastError());
        }
    #else
        SayHelloFunc sayHello = reinterpret_cast<SayHelloFunc>(dlsym(libHandle, "sayHello"));
        if (!sayHello) {
            Out::error("Failed to find function 'sayHello' in the library (Error: %s)", dlerror());
        }
    #endif

    if (sayHello) {
        Out::info("Calling 'sayHello' function from the library...");
        Scene scene;
        sayHello(&scene); // Call the function
    }
}