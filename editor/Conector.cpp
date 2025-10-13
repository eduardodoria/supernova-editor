#include "Conector.h"

#include "editor/Out.h"
#include "Scene.h"
#include "Project.h"

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
}

Editor::Conector::~Conector(){
    disconnect();
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

bool Editor::Conector::connect(const fs::path& buildPath, std::string libName){
    // Disconnect any existing connection first
    disconnect();

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
            Out::info("Successfully connected to library");
            return true;
        }
    } else {
        Out::error("Library file not found: %s", libPath.c_str());
    }

    return false;
}

void Editor::Conector::disconnect(){
    if (libHandle) {
        Out::info("Disconnecting from library...");

        // This allows the library to perform any necessary cleanup
        #ifdef _WIN32
            using CleanupFunc = void (*)();
            CleanupFunc cleanup = reinterpret_cast<CleanupFunc>(GetProcAddress(static_cast<HMODULE>(libHandle), "cleanup"));
            if (cleanup) {
                Out::info("Calling library cleanup function...");
                cleanup();
            }
        #else
            using CleanupFunc = void (*)();
            CleanupFunc cleanup = reinterpret_cast<CleanupFunc>(dlsym(libHandle, "cleanup"));
            if (cleanup) {
                Out::info("Calling library cleanup function...");
                cleanup();
            }
        #endif

        unloadSharedLibrary(libHandle);
        libHandle = nullptr;

        Out::info("Disconnected from library successfully");
    }
}

void Editor::Conector::execute(SceneProject* sceneProject){
    if (!libHandle) {
        Out::error("Cannot execute: Not connected to library");
        return;
    }

    // Dynamically load and call the `initScene` function
    using InitSceneFunc = void (*)(Scene*);
    #ifdef _WIN32
        InitSceneFunc initScene = reinterpret_cast<InitSceneFunc>(GetProcAddress(static_cast<HMODULE>(libHandle), "initScene"));
        if (!initScene) {
            Out::error("Failed to find function 'initScene' in the library (Error code: %i)", GetLastError());
        }
    #else
        InitSceneFunc initScene = reinterpret_cast<InitSceneFunc>(dlsym(libHandle, "initScene"));
        if (!initScene) {
            Out::error("Failed to find function 'initScene' in the library (Error: %s)", dlerror());
        }
    #endif

    if (initScene) {
        Out::info("Calling 'initScene' function from the library...");
        initScene(sceneProject->scene); // Call the function
    }
}

bool Editor::Conector::isLibraryConnected() const {
    return libHandle != nullptr;
}