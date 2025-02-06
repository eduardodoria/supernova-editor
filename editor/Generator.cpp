// Generator.cpp
#include "Generator.h"
#include "editor/Log.h"
#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include <fstream>

using namespace Supernova;

Editor::Generator::Generator() {
}

Editor::Generator::~Generator() {
    waitForBuildToComplete();
}


bool Editor::Generator::configureCMake(const fs::path& projectPath) {
    fs::path buildPath = projectPath / "build";
    fs::path currentPath = fs::current_path();
    std::string cmakeCommand = "cmake ";
    char buffer[256];
    FILE* pipe = nullptr;

    #ifdef _WIN32
        // First, query available generators
        pipe = _popen("cmake --help", "r");
        if (!pipe) {
            Log::error("Failed to query CMake generators");
            return false;
        }

        std::string result;
        bool inGeneratorsList = false;
        std::string generator;

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            if (line.find("Generators") != std::string::npos) {
                inGeneratorsList = true;
                continue;
            }
            if (inGeneratorsList && line.find("Visual Studio") != std::string::npos) {
                // Pick the first Visual Studio generator found (newest version)
                size_t start = line.find('=') != std::string::npos ? line.find('=') + 2 : 2;
                generator = line.substr(start);
                if (!generator.empty() && generator.back() == '\n') {
                    generator.pop_back();
                }
                break;
            }
        }
        _pclose(pipe);

        if (!generator.empty()) {
            cmakeCommand += "-G \"" + generator + "\" ";
        } else {
            // Fallback to default generator if no Visual Studio found
            Log::warning("No Visual Studio generator found, using default generator");
        }
    #endif

    cmakeCommand += "-DCMAKE_BUILD_TYPE=Debug ";
    cmakeCommand += projectPath.string() + " ";
    cmakeCommand += "-B " + buildPath.string() + " ";
    cmakeCommand += "-DSUPERNOVA_LIB_DIR=\"" + currentPath.string() + "\"";

    Log::info("Configuring CMake project with command: %s", cmakeCommand.c_str());

    #ifdef _WIN32
        pipe = _popen((cmakeCommand + " 2>&1").c_str(), "r");
    #else
        pipe = popen((cmakeCommand + " 2>&1").c_str(), "r");
    #endif

    if (!pipe) {
        return false;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        Log::build("%s", line.c_str());
    }

    #ifdef _WIN32
        int returnCode = _pclose(pipe);
    #else
        int returnCode = pclose(pipe);
    #endif

    return returnCode == 0;
}

bool Editor::Generator::buildProject(const fs::path& projectPath) {
    fs::path buildPath = projectPath / "build";

    std::string buildCommand = "cmake --build " + buildPath.string() + " --config Debug";

    Log::info("Building project...");

    #ifdef _WIN32
        FILE* pipe = _popen((buildCommand + " 2>&1").c_str(), "r");
    #else
        FILE* pipe = popen((buildCommand + " 2>&1").c_str(), "r");
    #endif

    if (!pipe) {
        return false;
    }

    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (!line.empty() && line.back() == '\n') {
            line.pop_back(); // Remove trailing newline
        }
        Log::build("%s", line.c_str());
    }

    #ifdef _WIN32
        int returnCode = _pclose(pipe);
    #else
        int returnCode = pclose(pipe);
    #endif

    return returnCode == 0;
}

bool Editor::Generator::writeIfChanged(const fs::path& filePath, const std::string& newContent) {
    std::string currentContent;
    bool shouldWrite = true;

    if (fs::exists(filePath)) {
        std::ifstream ifs(filePath);
        if (ifs) {
            currentContent = std::string(
                std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>()
            );
            shouldWrite = (currentContent != newContent);
        }
    }

    if (shouldWrite) {
        std::ofstream ofs(filePath);
        ofs << newContent;
        ofs.close();
        return true;
    }
    return false;
}

void Editor::Generator::writeSourceFiles(const fs::path& projectPath){
    const std::string cmakeContent = R"(
        cmake_minimum_required(VERSION 3.15)
        project(ProjectLib)

        # Specify C++ standard
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)

        # Project library target
        add_library(project_lib SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/project_lib.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/CubeScript.cpp
        )

        target_include_directories(project_lib PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/libs/sokol
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/libs/box2d/include
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/libs/joltphysics
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/renders
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core/ecs
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core/object
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core/object/physics
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core/script
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine/core/math
        )

        # Find supernova library in specified location
        find_library(SUPERNOVA_LIB supernova PATHS ${SUPERNOVA_LIB_DIR} NO_DEFAULT_PATH)
        if(NOT SUPERNOVA_LIB)
            message(FATAL_ERROR "Supernova library not found in ${SUPERNOVA_LIB_DIR}")
        endif()

        target_link_libraries(project_lib PRIVATE ${SUPERNOVA_LIB})

        # Set compile options based on compiler and platform
        if(MSVC)
            target_compile_options(project_lib PRIVATE /W4 /EHsc)
        else()
            target_compile_options(project_lib PRIVATE -Wall -Wextra -fPIC)
        endif()

        # Set properties for the shared library
        set_target_properties(project_lib PROPERTIES
            OUTPUT_NAME "project_lib"
            PREFIX ""
        )
    )";

    const std::string sourceContent = R"(
        #include <iostream>
        #include "Scene.h"
        #include "CubeScript.h"
        // A sample function to be called from the shared library
        extern "C" void sayHello(Supernova::Scene* scene) {
            //Supernova::Scene scene;
            CubeScript* cube = new CubeScript(scene);
            std::cout << "Hello from the shared library!" << std::endl;
        }
    )";

    const fs::path cmakeFile = projectPath / "CMakeLists.txt";
    const fs::path sourceFile = projectPath / "project_lib.cpp";

    bool cmakeChanged = writeIfChanged(cmakeFile, cmakeContent);
    bool sourceChanged = writeIfChanged(sourceFile, sourceContent);
}

void Editor::Generator::build(fs::path projectPath) {

    writeSourceFiles(projectPath);

    waitForBuildToComplete();

    buildFuture = std::async(std::launch::async, [this, projectPath]() {
        try {
            auto startTime = std::chrono::steady_clock::now();

            if (!configureCMake(projectPath)) {
                throw std::runtime_error("CMake configuration failed");
            }

            if (!buildProject(projectPath)) {
                throw std::runtime_error("Build failed");
            }

            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (endTime - startTime).count() / 1000.0;

            Log::build("Build completed in %.3f seconds", duration);
        } catch (const std::exception& ex) {
            Log::error("%s", ex.what());
        }
    });
}

bool Editor::Generator::isBuildInProgress() const {
    return buildFuture.valid() && 
           buildFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

void Editor::Generator::waitForBuildToComplete() {
    if (buildFuture.valid()) {
        buildFuture.wait();
    }
}