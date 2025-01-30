#include "Generator.h"

#include "editor/Log.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <array>
#include <memory>

#include <chrono>
#include <iomanip>
#include <regex>

#include <future>

using namespace Supernova;

Editor::Generator::Generator(){
}

Editor::Generator::~Generator() {
    waitForBuildToComplete();
}

// Helper function to execute a command and capture its output
std::string Editor::Generator::executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;

    struct PipeDeleter {
        void operator()(FILE* pipe) {
            if (pipe) {
                #ifdef _WIN32
                _pclose(pipe);
                #else
                pclose(pipe);
                #endif
            }
        }
    };

    #ifdef _WIN32
    std::unique_ptr<FILE, PipeDeleter> pipe(_popen(command.c_str(), "r"));
    #else
    std::unique_ptr<FILE, PipeDeleter> pipe(popen(command.c_str(), "r"));
    #endif

    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

// Function to detect the operating system
std::string Editor::Generator::getOperatingSystem() {
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#elif __APPLE__
    return "MacOS";
#else
    return "Unknown";
#endif
}

// Function to get compiler version
std::string Editor::Generator::getCompilerVersion(const std::string& compiler) {
    try {
        std::string fullOutput = executeCommand(compiler + " --version");

        // Check if it's MSVC
        if (compiler.find("cl") != std::string::npos) {
            // MSVC format: "Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30147 for x64"
            size_t pos = fullOutput.find("Version");
            if (pos != std::string::npos) {
                size_t end = fullOutput.find(" for", pos);
                return fullOutput.substr(pos + 8, end - (pos + 8));
            }
        }
        // Check if it's Clang
        else if (fullOutput.find("clang version") != std::string::npos) {
            // Clang format: "clang version 16.0.0"
            size_t pos = fullOutput.find("clang version");
            if (pos != std::string::npos) {
                size_t start = pos + 13; // Length of "clang version "
                size_t end = fullOutput.find('\n', start);
                return fullOutput.substr(start, end - start);
            }
        }
        // GCC format
        else {
            size_t pos = fullOutput.find(')');
            if (pos != std::string::npos) {
                return fullOutput.substr(pos + 2, fullOutput.find('\n') - pos - 2);
            }
            return fullOutput.substr(0, fullOutput.find('\n'));
        }
        return "Unknown";
    } catch (...) {
        return "Version unavailable";
    }
}

// Function to locate a compiler on the system
std::string Editor::Generator::findCompiler() {
    // Check cache first
    if (!compilerCache.empty()) {
        return compilerCache.begin()->second;
    }

    #ifdef _WIN32
        // Check for MSVC first on Windows
        if (std::system("cl 2>&1 | findstr \"Microsoft\" >nul 2>&1") == 0) {
            compilerCache["cl"] = "cl";
            return "cl";
        }
        std::vector<std::string> compilers = {"g++", "clang++"};
        std::string whereCmd = "where ";
    #else
        std::vector<std::string> compilers = {"g++", "clang++"};
        std::string whereCmd = "which ";
    #endif

    for (const auto& compiler : compilers) {
        std::string command = whereCmd + compiler;
        #ifndef _WIN32
            command += " 2>/dev/null";
        #endif
        try {
            std::string output = executeCommand(command);
            if (!output.empty()) {
                std::string path = output.substr(0, output.find('\n'));
                compilerCache[compiler] = path;
                return path;
            }
        } catch (...) {
            continue;
        }
    }
    throw std::runtime_error("No compiler found on the system!");
}

// Function to build a shared library
void Editor::Generator::buildSharedLibrary(
    const std::string& compiler,
    const std::vector<std::string>& sourceFiles,
    const std::string& outputFile,
    const std::vector<std::string>& includeDirs,
    bool debug,
    const fs::path& libPath,
    const std::vector<std::string>& libraries,
    const std::vector<std::string>& additionalFlags) {

    // Verify all source files exist
    for (const auto& sourceFile : sourceFiles) {
        if (!std::filesystem::exists(sourceFile)) {
            throw std::runtime_error("Source file does not exist: " + sourceFile);
        }
    }

    std::string os = getOperatingSystem();
    std::string optimizationFlags = debug ? "-O0 -g" : "-O3";
    std::string includeFlags;
    std::vector<std::string> objectFiles;

    // Build include directory flags
    if (compiler == "cl") {
        for (const auto& dir : includeDirs) {
            includeFlags += "/I\"" + dir + "\" ";
        }
    } else {
        for (const auto& dir : includeDirs) {
            includeFlags += "-I\"" + dir + "\" ";
        }
    }

    auto startTime = std::chrono::steady_clock::now();
    size_t fileCount = 0;
    const size_t totalFiles = sourceFiles.size();

    // First compile each source file to object files
    for (const auto& sourceFile : sourceFiles) {
        fileCount++;
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime).count() / 1000.0;

        // Calculate percentage
        int percentage = static_cast<int>((fileCount * 100) / totalFiles);

        // Create object file name
        std::string objFile = sourceFile;
        size_t lastDot = objFile.find_last_of('.');
        if (lastDot != std::string::npos) {
            objFile = objFile.substr(0, lastDot);
        }
        objFile += ".o";
        objectFiles.push_back(objFile);

        std::string compileCommand;
        if (compiler == "cl") {
            compileCommand = "cl /c /EHsc /O2 " + includeFlags + " " + sourceFile + 
                           " /Fo:" + objFile;
        } else {
            compileCommand = compiler + " -std=c++17 -fPIC -c " + optimizationFlags + 
                           " " + includeFlags + " " + sourceFile + " -o " + objFile;
        }

        Log::build("[%zu/%zu %3d%% :: %.3f] Building %s", 
               fileCount, totalFiles, percentage, duration, 
               sourceFile.c_str());

        int retCode = std::system(compileCommand.c_str());
        if (retCode != 0) {
            // Cleanup object files
            for (const auto& obj : objectFiles) {
                std::filesystem::remove(obj);
            }
            throw std::runtime_error("Failed to compile: " + sourceFile);
        }
    }

    // Now link all object files
    std::string linkCommand;
    if (compiler == "cl") {
        linkCommand = "cl /LD ";
        for (const auto& obj : objectFiles) {
            linkCommand += obj + " ";
        }
        linkCommand += "/Fe:" + outputFile + " /link /LIBPATH:\"" + libPath.string() + "\"";
        for (const auto& lib : libraries) {
            linkCommand += " " + lib + ".lib";
        }
    } else {
        linkCommand = compiler + " -shared ";
        for (const auto& obj : objectFiles) {
            linkCommand += obj + " ";
        }
        linkCommand += " -o " + outputFile;
        linkCommand += " -L\"" + libPath.string() + "\"";
        for (const auto& lib : libraries) {
            linkCommand += " -l" + lib;
        }
        if (os == "Linux") {
            linkCommand += " -Wl,-rpath,\"" + libPath.string() + "\"";
        } else if (os == "MacOS") {
            linkCommand += " -Wl,-rpath,@loader_path/\"" + libPath.string() + "\"";
        }
    }

    // Add additional flags to link command
    for (const auto& flag : additionalFlags) {
        linkCommand += " " + flag;
    }

    Log::build("Linking shared library...");
    int linkRetCode = std::system(linkCommand.c_str());

    // Cleanup object files
    for (const auto& obj : objectFiles) {
        std::filesystem::remove(obj);
    }

    if (linkRetCode != 0) {
        throw std::runtime_error("Failed to link shared library!");
    }

    auto endTime = std::chrono::steady_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
    Log::build("Build completed in %.3f seconds", totalDuration);
}

void Editor::Generator::build(fs::path projectPath) {
    try {
        std::string code = R"(
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

        std::string sourceFile = (projectPath / "project_lib.cpp").string();
        std::ofstream ofs(sourceFile);
        ofs << code;
        ofs.close();

        std::vector<std::string> sourceFiles;
        sourceFiles.push_back(sourceFile);
        sourceFiles.push_back((projectPath / "CubeScript.cpp").string());

        std::string outputFile = "project_lib";
        #ifdef _WIN32
            outputFile = outputFile + ".dll";
        #else
            outputFile = outputFile + ".so";
        #endif

        std::vector<std::string> includeDirs;

        includeDirs.push_back((projectPath / "supernova" / "engine" / "libs" /  "sokol").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "libs" /  "box2d" / "include").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "libs" /  "joltphysics").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "renders").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core" /  "ecs").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core" / "object").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core" / "object" / "physics").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core" / "script").string());
        includeDirs.push_back((projectPath / "supernova" / "engine" / "core" / "math").string());

        bool debug = true;

        std::cout << "Detecting operating system..." << std::endl;
        std::string os = getOperatingSystem();
        Log::info("Operating System: %s", os.c_str());

        std::cout << "Locating a compiler..." << std::endl;
        std::string compiler = findCompiler();
        Log::info("Found compiler: %s", compiler.c_str());

        std::string version = getCompilerVersion(compiler);
        Log::info("Compiler version: %s", version.c_str());

        Log::info("Building shared library...");
        std::vector<std::string> additionalFlags;

        // Wait for any existing build to complete
        waitForBuildToComplete();

        // Launch buildSharedLibrary in a separate thread
        buildFuture = std::async(std::launch::async, 
            [this, compiler, sourceFiles, outputFile, includeDirs, debug, projectPath]() {
                try {
                    buildSharedLibrary(compiler, sourceFiles, projectPath / outputFile, includeDirs, debug, "", {"supernova"}, {});
                    Log::info("Shared library built successfully: %s", outputFile.c_str());
                } catch (const std::exception& ex) {
                    Log::error("%s", ex.what());
                }
            });

    } catch (const std::exception& ex) {
        Log::error("%s", ex.what());
    }
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