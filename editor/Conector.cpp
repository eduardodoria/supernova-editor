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
            Out::error("Failed to load library: %s (Error code: %lu)", libPath.c_str(), GetLastError());
            return nullptr;
        }
        return libHandle;
    #else
        void* libHandle = dlopen(libPath.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!libHandle) {
            Out::error("Failed to load library: %s (Error: %s)", libPath.c_str(), dlerror());
            return nullptr;
        }
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

    fs::path fullLibPath = buildPath / fs::path(libPath);
    if (fileExists(fullLibPath)) {
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
        unloadSharedLibrary(libHandle);
        libHandle = nullptr;

        Out::info("Disconnected from library successfully");
    }
}

void Editor::Conector::cleanup(Supernova::Scene* scene){
    if (!libHandle) {
        Out::error("Cannot cleanup: Not connected to library");
        return;
    }

    using CleanupFunc = void (*)(Scene*);
    #ifdef _WIN32
        CleanupFunc cleanupFn = reinterpret_cast<CleanupFunc>(GetProcAddress(static_cast<HMODULE>(libHandle), "cleanup"));
        if (!cleanupFn) {
            Out::error("Failed to find function 'cleanup' in the library (Error code: %lu)", GetLastError());
        }
    #else
        dlerror(); // clear any existing error
        CleanupFunc cleanupFn = reinterpret_cast<CleanupFunc>(dlsym(libHandle, "cleanup"));
        const char* err = dlerror();
        if (err) {
            Out::error("Failed to find function 'cleanup' in the library (Error: %s)", err);
            cleanupFn = nullptr;
        }
    #endif

    if (cleanupFn) {
        try {
            cleanupFn(scene);
        } catch (const std::exception& e) {
            Out::error("Exception in cleanup(): %s", e.what());
        } catch (...) {
            Out::error("Unknown exception in cleanup()");
        }
    } else {
        Out::warning("Cleanup function not found in library");
    }
}

void Editor::Conector::execute(Supernova::Scene* scene){
    if (!libHandle) {
        Out::error("Cannot execute: Not connected to library");
        return;
    }

    using InitSceneFunc = void (*)(Scene*);
    #ifdef _WIN32
        InitSceneFunc initSceneFn = reinterpret_cast<InitSceneFunc>(GetProcAddress(static_cast<HMODULE>(libHandle), "initScene"));
        if (!initSceneFn) {
            Out::error("Failed to find function 'initScene' in the library (Error code: %lu)", GetLastError());
        }
    #else
        dlerror(); // clear any existing error
        InitSceneFunc initSceneFn = reinterpret_cast<InitSceneFunc>(dlsym(libHandle, "initScene"));
        const char* err = dlerror();
        if (err) {
            Out::error("Failed to find function 'initScene' in the library (Error: %s)", err);
            initSceneFn = nullptr;
        }
    #endif

    if (initSceneFn) {
        try {
            initSceneFn(scene);
        } catch (const std::exception& e) {
            Out::error("Exception in initScene(): %s", e.what());
        } catch (...) {
            Out::error("Unknown exception in initScene()");
        }
    }
}

bool Editor::Conector::isLibraryConnected() const {
    return libHandle != nullptr;
}