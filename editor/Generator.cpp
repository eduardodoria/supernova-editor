#include "Generator.h"

#include "editor/Log.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <thread>
#include <array>
#include <memory>

using namespace Supernova;

Editor::Generator::Generator(){
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
        return executeCommand(compiler + " --version");
    } catch (...) {
        return "Version information unavailable";
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
    bool debug,
    const std::vector<std::string>& additionalFlags) {

    // Verify all source files exist
    for (const auto& sourceFile : sourceFiles) {
        if (!std::filesystem::exists(sourceFile)) {
            throw std::runtime_error("Source file does not exist: " + sourceFile);
        }
    }

    std::string os = getOperatingSystem();
    std::string command;
    std::string optimizationFlags = debug ? "-O0 -g" : "-O3";

    if (compiler == "cl") {
        // MSVC compilation command
        command = "cl /LD /EHsc /O2 ";
        for (const auto& sourceFile : sourceFiles) {
            command += sourceFile + " ";
        }
        command += "/Fe:" + outputFile;
    } else {
        // GCC/Clang compilation command
        if (os == "Linux" || os == "MacOS") {
            command = compiler + " -std=c++17 -fPIC -shared " + optimizationFlags;
        } else if (os == "Windows") {
            command = compiler + " -std=c++17 -shared " + optimizationFlags;
        } else {
            throw std::runtime_error("Unsupported operating system: " + os);
        }

        // Add source files
        for (const auto& sourceFile : sourceFiles) {
            command += " " + sourceFile;
        }

        // Add output file
        command += " -o " + outputFile;
    }

    // Add additional flags
    for (const auto& flag : additionalFlags) {
        command += " " + flag;
    }

    std::cout << "Building shared library with command: " << command << std::endl;

    int retCode = std::system(command.c_str());
    if (retCode != 0) {
        throw std::runtime_error("Failed to build shared library!");
    }
}

void Editor::Generator::build() {
    try {
        std::string code = R"(#include <iostream>
            // A sample function to be called from the shared library
            extern "C" void sayHello() {
                std::cout << "Hello from the shared library again!" << std::endl;
            })";

        std::string sourceFile = "project_lib.cpp";
        std::ofstream ofs(sourceFile);
        ofs << code;
        ofs.close();

        std::vector<std::string> sourceFiles;
        sourceFiles.push_back(sourceFile);

        std::string outputFile = "project_lib";
        #ifdef _WIN32
            outputFile = outputFile + ".dll";
        #else
            outputFile = outputFile + ".so";
        #endif

        bool debug = true;

        std::cout << "Detecting operating system..." << std::endl;
        std::string os = getOperatingSystem();
        Log::info("Operating System: %s", os.c_str());

        std::cout << "Locating a compiler..." << std::endl;
        std::string compiler = findCompiler();
        Log::info("Found compiler: %s", compiler.c_str());

        std::string version = getCompilerVersion(compiler);
        Log::info("Compiler version:\n %s", version.c_str());

        Log::info("Building shared library...");
        std::vector<std::string> additionalFlags;
        buildSharedLibrary(compiler, sourceFiles, outputFile, debug, additionalFlags);

        Log::info("Shared library built successfully: %s", outputFile.c_str());
    } catch (const std::exception& ex) {
        Log::error("%s", ex.what());
    }
}