// Generator.cpp
#include "Generator.h"
#include "editor/Out.h"
#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace Supernova;

Editor::Generator::Generator() {
}

Editor::Generator::~Generator() {
    waitForBuildToComplete();
}

bool Editor::Generator::configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType) {
    fs::path currentPath = fs::current_path();
    std::string cmakeCommand = "cmake ";
    char buffer[4096];

    #ifdef _WIN32
        cmakeCommand += "-G \"Visual Studio 17 2022\" ";
    #endif

    cmakeCommand += "-DCMAKE_BUILD_TYPE=" + configType  + " ";
    cmakeCommand += "\"" + projectPath.string() + "\" ";
    cmakeCommand += "-B \"" + buildPath.string() + "\" ";
    cmakeCommand += "-DSUPERNOVA_LIB_DIR=\"" + currentPath.string() + "\"";

    Out::info("Configuring CMake project with command: %s", cmakeCommand.c_str());

    #ifdef _WIN32
        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hReadPipe, hWritePipe;

        // Create pipe for redirecting output
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            return false;
        }

        // Set up startup info for the process
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.hStdInput = NULL;

        // Create the process
        std::string cmdLine = "cmd.exe /c " + cmakeCommand;
        BOOL success = CreateProcessA(
            NULL,                           // No module name (use command line)
            (LPSTR)cmdLine.c_str(),        // Command line
            NULL,                           // Process handle not inheritable
            NULL,                           // Thread handle not inheritable
            TRUE,                           // Handle inheritance required for pipes
            CREATE_NO_WINDOW,               // Creation flags
            NULL,                           // Use parent's environment block
            projectPath.string().c_str(),   // Set working directory
            &si,                            // Startup info
            &pi                             // Process information
        );

        if (!success) {
            DWORD error = GetLastError();
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            Log::error("Failed to create process. Error code: %lu", error);
            return false;
        }

        // Close the write end of the pipe as we don't need it
        CloseHandle(hWritePipe);

        // Read output from the pipe
        DWORD bytesRead;
        std::string accumulator;
        bool processFinished = false;
        DWORD exitCode = 0;

        while (!processFinished) {
            // Check if the process has finished
            if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
                if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
                    processFinished = true;
                }
            }

            // Read any available output
            while (true) {
                DWORD bytesAvailable = 0;
                if (!PeekNamedPipe(hReadPipe, NULL, 0, NULL, &bytesAvailable, NULL) || bytesAvailable == 0) {
                    break;
                }

                if (!ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0) {
                    break;
                }

                buffer[bytesRead] = '\0';
                accumulator += buffer;

                // Process complete lines
                size_t pos = 0;
                size_t nextPos;
                while ((nextPos = accumulator.find('\n', pos)) != std::string::npos) {
                    std::string line = accumulator.substr(pos, nextPos - pos);
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();  // Remove trailing \r if present
                    }
                    if (!line.empty()) {
                        Log::build("%s", line.c_str());
                    }
                    pos = nextPos + 1;
                }

                // Keep any remaining partial line in the accumulator
                if (pos < accumulator.length()) {
                    accumulator = accumulator.substr(pos);
                } else {
                    accumulator.clear();
                }
            }

            // Small delay to prevent high CPU usage
            Sleep(10);
        }

        // Process any remaining output
        if (!accumulator.empty()) {
            Log::build("%s", accumulator.c_str());
        }

        // Clean up handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadPipe);

        return exitCode == 0;
    #else
        FILE* pipe = popen((cmakeCommand + " 2>&1").c_str(), "r");
        if (!pipe) {
            return false;
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }
            Out::build("%s", line.c_str());
        }

        return pclose(pipe) == 0;
    #endif
}

bool Editor::Generator::buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType) {
    std::string buildCommand = "cmake --build \"" + buildPath.string() + "\" --config " + configType;
    char buffer[4096];

    Out::info("Building project...");

    #ifdef _WIN32
        // avoid warning to building in a temporary directory
        SetEnvironmentVariableA("MSBuildWarningsAsMessages", "MSB8029");

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hReadPipe, hWritePipe;

        // Create pipe for redirecting output
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            return false;
        }

        // Set up startup info for the process
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;
        si.hStdInput = NULL;

        // Create the process
        std::string cmdLine = "cmd.exe /c " + buildCommand;
        BOOL success = CreateProcessA(
            NULL,                           // No module name (use command line)
            (LPSTR)cmdLine.c_str(),        // Command line
            NULL,                           // Process handle not inheritable
            NULL,                           // Thread handle not inheritable
            TRUE,                           // Handle inheritance required for pipes
            CREATE_NO_WINDOW,               // Creation flags
            NULL,                           // Use parent's environment block
            buildPath.string().c_str(),     // Set working directory to build path
            &si,                            // Startup info
            &pi                             // Process information
        );

        if (!success) {
            DWORD error = GetLastError();
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            Log::error("Failed to create process. Error code: %lu", error);
            return false;
        }

        // Close the write end of the pipe as we don't need it
        CloseHandle(hWritePipe);

        // Read output from the pipe
        DWORD bytesRead;
        std::string accumulator;
        bool processFinished = false;
        DWORD exitCode = 0;

        while (!processFinished) {
            // Check if the process has finished
            if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
                if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
                    processFinished = true;
                }
            }

            // Read any available output
            while (true) {
                DWORD bytesAvailable = 0;
                if (!PeekNamedPipe(hReadPipe, NULL, 0, NULL, &bytesAvailable, NULL) || bytesAvailable == 0) {
                    break;
                }

                if (!ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0) {
                    break;
                }

                buffer[bytesRead] = '\0';
                accumulator += buffer;

                // Process complete lines
                size_t pos = 0;
                size_t nextPos;
                while ((nextPos = accumulator.find('\n', pos)) != std::string::npos) {
                    std::string line = accumulator.substr(pos, nextPos - pos);
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();  // Remove trailing \r if present
                    }
                    if (!line.empty()) {
                        Log::build("%s", line.c_str());
                    }
                    pos = nextPos + 1;
                }

                // Keep any remaining partial line in the accumulator
                if (pos < accumulator.length()) {
                    accumulator = accumulator.substr(pos);
                } else {
                    accumulator.clear();
                }
            }

            // Small delay to prevent high CPU usage
            Sleep(10);
        }

        // Process any remaining output
        if (!accumulator.empty()) {
            Log::build("%s", accumulator.c_str());
        }

        // Clean up handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadPipe);

        return exitCode == 0;
    #else
        FILE* pipe = popen((buildCommand + " 2>&1").c_str(), "r");
        if (!pipe) {
            return false;
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }
            Out::build("%s", line.c_str());
        }

        return pclose(pipe) == 0;
    #endif
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

        # To suppress warnings if not Debug
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(SUPERNOVA_LIB_SYSTEM SYSTEM)
        endif()

        target_include_directories(project_lib ${SUPERNOVA_LIB_SYSTEM} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/supernova/engine
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
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}
            OUTPUT_NAME "project_lib"
            PREFIX ""
        )
    )";

    const std::string sourceContent = R"(
        #include <iostream>
        #include "Scene.h"
        #include "CubeScript.h"

        #if defined(_MSC_VER)
            #define PROJECT_API __declspec(dllexport)
        #else
            #define PROJECT_API
        #endif

        // A sample function to be called from the shared library
        extern "C" void PROJECT_API sayHello(Supernova::Scene* scene) {
            //Supernova::Scene scene;
            CubeScript* cube = new CubeScript(scene);
            cube->init();
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

            fs::path buildPath = projectPath / "build";

            #ifdef _DEBUG
                std::string configType = "Debug";
            #else
                std::string configType = "Release";
            #endif

            if (!configureCMake(projectPath, buildPath, configType)) {
                throw std::runtime_error("CMake configuration failed");
            }

            if (!buildProject(projectPath, buildPath, configType)) {
                throw std::runtime_error("Build failed");
            }

            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (endTime - startTime).count() / 1000.0;

            Out::build("Build completed in %.3f seconds", duration);
        } catch (const std::exception& ex) {
            Out::error("%s", ex.what());
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