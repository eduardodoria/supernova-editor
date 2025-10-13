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
            Out::error("Failed to create process. Error code: %lu", error);
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
                        Out::build("%s", line.c_str());
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
            Out::build("%s", accumulator.c_str());
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
            Out::error("Failed to create process. Error code: %lu", error);
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
                        Out::build("%s", line.c_str());
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
            Out::build("%s", accumulator.c_str());
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

bool Editor::Generator::tryIncludeHeader(const fs::path& p, const fs::path& projectPath, std::unordered_set<std::string>& included, std::string& sourceContent) {
    std::string ext = p.extension().string();
    if (ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx") {
        fs::path relativePath = fs::relative(p, projectPath);
        std::string inc = relativePath.generic_string();
        if (included.insert(inc).second) {
            sourceContent += "#include \"" + inc + "\"\n";
        }
        return true;
    }
    // Try to guess header from a source file name
    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") {
        fs::path h = p; h.replace_extension(".h");
        if (fs::exists(h)) {
            fs::path relativePath = fs::relative(h, projectPath);
            std::string inc = relativePath.generic_string();
            if (included.insert(inc).second) {
                sourceContent += "#include \"" + inc + "\"\n";
            }
            return true;
        }
        fs::path hpp = p; hpp.replace_extension(".hpp");
        if (fs::exists(hpp)) {
            fs::path relativePath = fs::relative(hpp, projectPath);
            std::string inc = relativePath.generic_string();
            if (included.insert(inc).second) {
                sourceContent += "#include \"" + inc + "\"\n";
            }
            return true;
        }
    }
    return false;
}

void Editor::Generator::writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<ScriptSource>& scriptFiles){
    // Get the project internal path for engine API includes
    fs::path relativeInternalPath = fs::relative(projectInternalPath, projectPath);
    fs::path engineApiRelativePath = relativeInternalPath / "engine-api";

    std::string internalPathStr = "${CMAKE_CURRENT_SOURCE_DIR}/" + relativeInternalPath.generic_string();
    std::string engineApiPathStr = "${CMAKE_CURRENT_SOURCE_DIR}/" + engineApiRelativePath.generic_string();

    // Build SCRIPT_SOURCES list for CMake
    std::string scriptDirs = "";
    std::string scriptSources = "set(SCRIPT_SOURCES\n";
    for (const auto& s : scriptFiles) {
        // Make path relative to project path and add CMake variable prefix
        fs::path relativePath = fs::relative(s.path, projectPath);
        scriptSources += "    ${CMAKE_CURRENT_SOURCE_DIR}/" + relativePath.generic_string() + "\n";

        if (!s.headerPath.empty()) {
            fs::path relativeHeaderPath = fs::relative(s.headerPath, projectPath);
            fs::path relativeDir = relativeHeaderPath.parent_path();
            scriptDirs += "    ${CMAKE_CURRENT_SOURCE_DIR}/" + relativeDir.generic_string() + "\n";
        }
    }
    scriptSources += ")\n";

    std::string cmakeContent;
    cmakeContent += "cmake_minimum_required(VERSION 3.15)\n";
    cmakeContent += "project(ProjectLib)\n\n";
    cmakeContent += "# Specify C++ standard\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD 17)\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    cmakeContent += scriptSources + "\n";
    cmakeContent += "# Project library target\n";
    cmakeContent += "add_library(" + libName + " SHARED\n";
    cmakeContent += "    " + internalPathStr + "/project_main.cpp\n";
    cmakeContent += "    ${SCRIPT_SOURCES}\n";
    cmakeContent += ")\n\n";
    cmakeContent += "# To suppress warnings if not Debug\n";
    cmakeContent += "if(NOT CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
    cmakeContent += "    set(SUPERNOVA_LIB_SYSTEM SYSTEM)\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "target_include_directories(" + libName + " ${SUPERNOVA_LIB_SYSTEM} PRIVATE\n";
    cmakeContent += scriptDirs + "\n";
    cmakeContent += "    " + engineApiPathStr + "\n";
    cmakeContent += "    " + engineApiPathStr + "/libs/sokol\n";
    cmakeContent += "    " + engineApiPathStr + "/libs/box2d/include\n";
    cmakeContent += "    " + engineApiPathStr + "/libs/joltphysics\n";
    cmakeContent += "    " + engineApiPathStr + "/renders\n";
    cmakeContent += "    " + engineApiPathStr + "/core\n";
    cmakeContent += "    " + engineApiPathStr + "/core/ecs\n";
    cmakeContent += "    " + engineApiPathStr + "/core/object\n";
    cmakeContent += "    " + engineApiPathStr + "/core/object/physics\n";
    cmakeContent += "    " + engineApiPathStr + "/core/script\n";
    cmakeContent += "    " + engineApiPathStr + "/core/math\n";
    cmakeContent += "    " + engineApiPathStr + "/core/registry\n";
    cmakeContent += "    " + engineApiPathStr + "/core/util\n";
    cmakeContent += "    " + engineApiPathStr + "/core/component\n";
    cmakeContent += ")\n\n";
    cmakeContent += "# Find supernova library in specified location\n";
    cmakeContent += "find_library(SUPERNOVA_LIB supernova PATHS ${SUPERNOVA_LIB_DIR} NO_DEFAULT_PATH)\n";
    cmakeContent += "if(NOT SUPERNOVA_LIB)\n";
    cmakeContent += "    message(FATAL_ERROR \"Supernova library not found in ${SUPERNOVA_LIB_DIR}\")\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "target_link_libraries(" + libName + " PRIVATE ${SUPERNOVA_LIB})\n\n";
    cmakeContent += "# Set compile options based on compiler and platform\n";
    cmakeContent += "if(MSVC)\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE /W4 /EHsc)\n";
    cmakeContent += "else()\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE -Wall -Wextra -fPIC)\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "# Set properties for the shared library\n";
    cmakeContent += "set_target_properties(" + libName + " PROPERTIES\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    OUTPUT_NAME \"" + libName + "\"\n";
    cmakeContent += "    PREFIX \"\"\n";
    cmakeContent += ")\n";

    // Build C++ source content
    std::string sourceContent;
    sourceContent += "#include <vector>\n";
    sourceContent += "#include <string>\n";
    sourceContent += "#include \"Scene.h\"\n";
    sourceContent += "#include \"EntityHandle.h\"\n";
    sourceContent += "#include \"ScriptComponent.h\"\n";

    // Include script headers
    std::unordered_set<std::string> included;
    for (const auto& s : scriptFiles) {
        tryIncludeHeader(s.path, projectPath, included, sourceContent);
    }

    sourceContent += "\n";
    sourceContent += "#if defined(_MSC_VER)\n";
    sourceContent += "    #define PROJECT_API __declspec(dllexport)\n";
    sourceContent += "#else\n";
    sourceContent += "    #define PROJECT_API\n";
    sourceContent += "#endif\n\n";

    sourceContent += "static std::vector<Supernova::EntityHandle*> g_scriptInstances;\n\n";

    sourceContent += "extern \"C\" void PROJECT_API initScene(Supernova::Scene* scene) {\n";

    // Build script instantiation code with direct property assignments
    for (size_t i = 0; i < scriptFiles.size(); ++i) {
        const auto& s = scriptFiles[i];

        sourceContent += "    {\n";
        sourceContent += "        " + s.className + "* script = new " + s.className + "(scene, (Supernova::Entity)" + std::to_string(s.entity) + ");\n";
        sourceContent += "        \n";

        // Get ScriptComponent from the scene to retrieve current property values
        ScriptComponent* scriptComp = s.scene->findComponent<ScriptComponent>(s.entity);
        if (scriptComp && !scriptComp->properties.empty()) {
            // Set memberPtr for each property
            sourceContent += "        Supernova::ScriptComponent* scriptComp = scene->findComponent<Supernova::ScriptComponent>((Supernova::Entity)" + std::to_string(s.entity) + ");\n";
            sourceContent += "        if (scriptComp) {\n";

            sourceContent += "            // Set member pointers for all properties\n";
            sourceContent += "            for (auto& prop : scriptComp->properties) {\n";

            // Generate if-else chain for property name matching
            bool first = true;
            for (const auto& prop : scriptComp->properties) {
                if (first) {
                    sourceContent += "                if (prop.name == \"" + prop.name + "\") {\n";
                    first = false;
                } else {
                    sourceContent += "                } else if (prop.name == \"" + prop.name + "\") {\n";
                }
                sourceContent += "                    prop.memberPtr = &script->" + prop.name + ";\n";
            }
            if (!scriptComp->properties.empty()) {
                sourceContent += "                }\n";
            }

            sourceContent += "            }\n";
            sourceContent += "            \n";
            sourceContent += "            // Sync stored values to member variables\n";
            sourceContent += "            for (auto& prop : scriptComp->properties) {\n";
            sourceContent += "                prop.syncToMember();\n";
            sourceContent += "            }\n";
            sourceContent += "        }\n";
            sourceContent += "        \n";
        }

        sourceContent += "        g_scriptInstances.push_back(script);\n";
        sourceContent += "    }\n";
    }

    if (scriptFiles.empty()) {
        sourceContent += "    (void)scene; // Suppress unused parameter warning\n";
    }

    sourceContent += "}\n\n";

    // Cleanup function that properly deletes through base class pointers
    sourceContent += "extern \"C\" void PROJECT_API cleanup() {\n";
    sourceContent += "    // Clean up all script instances (calls virtual destructors)\n";
    sourceContent += "    for (Supernova::EntityHandle* script : g_scriptInstances) {\n";
    sourceContent += "        delete script; // Properly calls virtual destructor\n";
    sourceContent += "    }\n";
    sourceContent += "    g_scriptInstances.clear();\n";
    sourceContent += "}\n";

    const fs::path cmakeFile = projectPath / "CMakeLists.txt";
    const fs::path sourceFile = projectInternalPath / "project_main.cpp";

    writeIfChanged(cmakeFile, cmakeContent);
    writeIfChanged(sourceFile, sourceContent);
}

void Editor::Generator::build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath, std::string libName, const std::vector<ScriptSource>& scriptFiles) {

    writeSourceFiles(projectPath, projectInternalPath, libName, scriptFiles);

    waitForBuildToComplete();

    lastBuildSucceeded = false; // Reset status before starting new build

    buildFuture = std::async(std::launch::async, [this, projectPath, buildPath]() {
        try {
            auto startTime = std::chrono::steady_clock::now();

            #ifdef _DEBUG
                std::string configType = "Debug";
            #else
                std::string configType = "Release";
            #endif

            if (!configureCMake(projectPath, buildPath, configType)) {
                Out::error("CMake configuration failed");
                lastBuildSucceeded = false;
                return;
            }

            if (!buildProject(projectPath, buildPath, configType)) {
                Out::error("Build failed");
                lastBuildSucceeded = false;
                return;
            }

            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                (endTime - startTime).count() / 1000.0;

            Out::build("Build completed successfully in %.3f seconds", duration);
            lastBuildSucceeded = true;
        } catch (const std::exception& ex) {
            Out::error("Build exception: %s", ex.what());
            lastBuildSucceeded = false;
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

bool Editor::Generator::didLastBuildSucceed() const {
    return lastBuildSucceeded;
}