// Generator.cpp
#include "Generator.h"
#include "Factory.h"
#include "editor/Out.h"

#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <thread>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <signal.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include <fcntl.h>

    extern char **environ;
#endif

using namespace Supernova;

namespace {
    constexpr std::chrono::milliseconds kReadSleepMs{10};
    constexpr std::chrono::milliseconds kKillGracePeriod{100};
}

fs::path Editor::Generator::getExecutableDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    return fs::path(path).parent_path();
#else
    return fs::canonical("/proc/self/exe").parent_path();
#endif
}

Editor::Generator::Generator() :
    lastBuildSucceeded(false),
    cancelRequested(false)
{
#ifdef _WIN32
    currentProcessHandle = NULL;
#else
    currentProcessPid = 0;
#endif
}

Editor::Generator::~Generator() {
    // Request cancellation and wait for it to finish before destroying
    try {
        auto f = cancelBuild();
        if (f.valid()) f.wait();
    } catch (...) {}
    waitForBuildToComplete();
}

bool Editor::Generator::configureCMake(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType) {
    const fs::path exePath = getExecutableDir();

    std::string cmakeCommand = "cmake ";
    #ifdef _WIN32
        cmakeCommand += "-G \"Visual Studio 17 2022\" ";
    #endif
    cmakeCommand += "-DCMAKE_BUILD_TYPE=" + configType + " ";
    // When configuring from inside the editor, ensure the generated project builds
    // in "plugin" mode (no Factory init.cpp/scene sources added).
    cmakeCommand += "-DSUPERNOVA_EDITOR_PLUGIN=ON ";
    cmakeCommand += "\"" + projectPath.string() + "\" ";
    cmakeCommand += "-B \"" + buildPath.string() + "\" ";
    cmakeCommand += "-DSUPERNOVA_LIB_DIR=\"" + exePath.string() + "\"";

    Out::info("Configuring CMake project with command: %s", cmakeCommand.c_str());
    return runCommand(cmakeCommand, projectPath);
}

bool Editor::Generator::buildProject(const fs::path& projectPath, const fs::path& buildPath, const std::string& configType) {
    std::string buildCommand = "cmake --build \"" + buildPath.string() + "\" --config " + configType;
    Out::info("Building project...");
    return runCommand(buildCommand, buildPath);
}

bool Editor::Generator::runCommand(const std::string& command, const fs::path& workingDir) {
    cancelRequested.store(false, std::memory_order_relaxed);

    #ifdef _WIN32
        SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        HANDLE hReadPipe = nullptr;
        HANDLE hWritePipe = nullptr;
        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
            Out::error("Failed to create pipe (Win32). Error code: %lu", GetLastError());
            return false;
        }
        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si{};
        si.cb = sizeof(STARTUPINFOA);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError  = hWritePipe;
        si.hStdInput  = nullptr;

        PROCESS_INFORMATION pi{};
        std::string cmdLine = "cmd.exe /c " + command;

        if (!CreateProcessA(
                nullptr,
                cmdLine.data(),
                nullptr,
                nullptr,
                TRUE,
                CREATE_NO_WINDOW,
                nullptr,
                workingDir.empty() ? nullptr : workingDir.string().c_str(),
                &si,
                &pi))
        {
            DWORD err = GetLastError();
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            Out::error("Failed to create process. Error code: %lu", err);
            return false;
        }

        CloseHandle(hWritePipe);

        {
            std::lock_guard<std::mutex> lock(processHandleMutex);
            currentProcessHandle = pi.hProcess;
        }

        constexpr size_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];
        std::string accumulator;

        DWORD exitCode = 1;
        bool finished = false;

        while (!finished) {
            if (cancelRequested.load(std::memory_order_relaxed)) {
                terminateCurrentProcess();
                CloseHandle(pi.hThread);
                {
                    std::lock_guard<std::mutex> lock(processHandleMutex);
                    currentProcessHandle = NULL;
                }
                CloseHandle(pi.hProcess);
                CloseHandle(hReadPipe);  // Close pipe at the end
                return false;
            }

            DWORD bytesAvailable = 0;
            while (PeekNamedPipe(hReadPipe, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0) {
                DWORD bytesRead = 0;
                if (!ReadFile(hReadPipe, buffer, static_cast<DWORD>(BUFFER_SIZE - 1), &bytesRead, nullptr) || bytesRead == 0) {
                    break;
                }

                buffer[bytesRead] = '\0';
                accumulator.append(buffer, bytesRead);

                size_t pos = 0;
                size_t nextPos = 0;
                while ((nextPos = accumulator.find('\n', pos)) != std::string::npos) {
                    std::string line = accumulator.substr(pos, nextPos - pos);
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    if (!line.empty()) {
                        Out::build("%s", line.c_str());
                    }
                    pos = nextPos + 1;
                }
                accumulator.erase(0, pos);
            }

            DWORD waitResult = WaitForSingleObject(pi.hProcess, 50);
            if (waitResult == WAIT_OBJECT_0) {
                finished = true;
                GetExitCodeProcess(pi.hProcess, &exitCode);
            } else if (waitResult == WAIT_FAILED) {
                finished = true;
            } else {
                std::this_thread::sleep_for(kReadSleepMs);
            }
        }

        if (!accumulator.empty()) {
            Out::build("%s", accumulator.c_str());
        }

        CloseHandle(hReadPipe);
        CloseHandle(pi.hThread);

        {
            std::lock_guard<std::mutex> lock(processHandleMutex);
            currentProcessHandle = NULL;
        }
        CloseHandle(pi.hProcess);

        return exitCode == 0;
    #else
        int pipefd[2];
        if (pipe(pipefd) != 0) {
            Out::error("Failed to create pipe (POSIX): %s", strerror(errno));
            return false;
        }

        // Make read end non-blocking
        int flags = fcntl(pipefd[0], F_GETFL, 0);
        fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_addclose(&actions, pipefd[0]);
        posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, pipefd[1]);

        pid_t pid = 0;
        std::string workDirStr = workingDir.string();
        std::string cdPrefix;
        if (!workDirStr.empty()) {
            cdPrefix = "cd \"" + workDirStr + "\" && ";
        }
        std::string shellCommand = cdPrefix + command;

        char *argv[] = { const_cast<char*>("/bin/sh"),
                        const_cast<char*>("-c"),
                        const_cast<char*>(shellCommand.c_str()),
                        nullptr };

        int spawnResult = posix_spawn(&pid, "/bin/sh", &actions, nullptr, argv, environ);
        posix_spawn_file_actions_destroy(&actions);
        close(pipefd[1]);

        if (spawnResult != 0) {
            close(pipefd[0]);
            Out::error("Failed to spawn process (POSIX): %s", strerror(spawnResult));
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(processPidMutex);
            currentProcessPid = pid;
        }

        constexpr size_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];
        bool processExited = false;

        while (!processExited) {
            // Check for cancellation
            if (cancelRequested.load(std::memory_order_relaxed)) {
                terminateCurrentProcess();
                processExited = true;
                break;
            }

            // Try to read with a short timeout
            ssize_t bytesRead = read(pipefd[0], buffer, BUFFER_SIZE - 1);

            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                std::string line(buffer);
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                if (!line.empty()) {
                    Out::build("%s", line.c_str());
                }
            } else if (bytesRead == 0) {
                // EOF - pipe closed
                processExited = true;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, check process status
                int status;
                pid_t result = waitpid(pid, &status, WNOHANG);
                if (result == pid) {
                    processExited = true;
                } else if (result == -1) {
                    processExited = true;
                }
                // Sleep briefly to avoid busy-waiting
                std::this_thread::sleep_for(kReadSleepMs);
            } else {
                // Read error
                Out::error("Error reading from pipe: %s", strerror(errno));
                processExited = true;
            }
        }

        // Always close the pipe after the loop
        close(pipefd[0]);

        int status = 0;
        while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {
            // retry on EINTR
        }

        {
            std::lock_guard<std::mutex> lock(processPidMutex);
            currentProcessPid = 0;
        }

        if (cancelRequested.load(std::memory_order_relaxed)) {
            return false;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status) == 0;
        }
        if (WIFSIGNALED(status)) {
            Out::warning("Process terminated by signal %d", WTERMSIG(status));
        }
        return false;
    #endif
}

bool Editor::Generator::writeIfChanged(const fs::path& filePath, const std::string& newContent) {
    std::string currentContent;
    bool shouldWrite = true;

    if (fs::exists(filePath)) {
        std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
        if (ifs) {
            currentContent.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            shouldWrite = (currentContent != newContent);
        }
    }

    if (shouldWrite) {
        std::error_code ec;
        if (filePath.has_parent_path()) {
            fs::create_directories(filePath.parent_path(), ec);
        }
        std::ofstream ofs(filePath, std::ios::out | std::ios::binary);
        ofs << newContent;
        return true;
    }
    return false;
}

std::string Editor::Generator::getPlatformCMakeConfig() {
    std::string content;
    content += "if (NOT SUPERNOVA_EDITOR_PLUGIN)\n";
    content += "    add_definitions(\"-DDEFAULT_WINDOW_WIDTH=960\")\n";
    content += "    add_definitions(\"-DDEFAULT_WINDOW_HEIGHT=540\")\n";
    content += "\n";
    content += "    set(COMPILE_ZLIB OFF)\n";
    content += "    set(IS_ARM OFF)\n";
    content += "\n";
    content += "    add_definitions(\"-DSOKOL_GLCORE\")\n";
    content += "    add_definitions(\"-DWITH_MINIAUDIO\") # For SoLoud\n";
    content += "\n";
    content += "    list(APPEND PLATFORM_SOURCE\n";
    content += "        ${INTERNAL_DIR}/generated/PlatformEditor.cpp\n";
    content += "        ${INTERNAL_DIR}/generated/main.cpp\n";
    content += "    )\n";
    content += "\n";
    content += "    list(APPEND PLATFORM_LIBS\n";
    content += "        GL dl m glfw\n";
    content += "    )\n";
    content += "endif() \n";
    return content;
}

void Editor::Generator::writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<ScriptSource>& scriptFiles, const std::vector<SceneData>& scenes) {
    const fs::path exePath = getExecutableDir();

    fs::path relativeInternalPath = fs::relative(projectInternalPath, projectPath);
    fs::path engineApiRelativePath = relativeInternalPath / "engine-api";

    std::string internalPathStr = "${CMAKE_CURRENT_SOURCE_DIR}/" + relativeInternalPath.generic_string();
    std::string engineApiPathStr = "${CMAKE_CURRENT_SOURCE_DIR}/" + engineApiRelativePath.generic_string();

    // Build FACTORY_SOURCES list for CMake (generated by Factory in configure())
    std::string factorySources = "set(FACTORY_SOURCES\n";
    factorySources += "    " + internalPathStr + "/generated/init.cpp\n";
    for (const auto& sceneData : scenes) {
        std::string filename = Factory::toIdentifier(sceneData.name) + ".cpp";
        factorySources += "    " + internalPathStr + "/generated/" + filename + "\n";
    }
    factorySources += ")\n";

    // Build SCRIPT_SOURCES list for CMake
    std::string scriptDirs = "";
    std::string scriptSources = "set(SCRIPT_SOURCES\n";
    for (const auto& s : scriptFiles) {
        if (s.path.is_relative()) {
            scriptSources += "    ${CMAKE_CURRENT_SOURCE_DIR}/" + s.path.generic_string() + "\n";
        } else {
            scriptSources += "    " + s.path.generic_string() + "\n";
        }

        if (!s.headerPath.empty() && s.headerPath.is_relative()) {
            fs::path relativeDir = s.headerPath.parent_path();
            scriptDirs += "    ${CMAKE_CURRENT_SOURCE_DIR}/" + relativeDir.generic_string() + "\n";
        }
    }
    scriptSources += ")\n";

    std::string cmakeContent;
    cmakeContent += "# This file is auto-generated by Supernova Editor. Do not edit manually.\n\n";
    cmakeContent += "cmake_minimum_required(VERSION 3.15)\n";
    cmakeContent += "project(ProjectLib)\n\n";
    cmakeContent += "set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})\n";
    cmakeContent += "set(INTERNAL_DIR ${PROJECT_ROOT}/.supernova)\n\n";

    cmakeContent += "# Specify C++ standard\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD 17)\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    cmakeContent += "# Build mode: when ON, build as Supernova Editor plugin (shared library)\n";
    cmakeContent += "option(SUPERNOVA_EDITOR_PLUGIN \"Build as Supernova Editor plugin\" OFF)\n";
    cmakeContent += "if(SUPERNOVA_EDITOR_PLUGIN)\n";
    cmakeContent += "    add_compile_definitions(SUPERNOVA_EDITOR_PLUGIN)\n";
    cmakeContent += "endif()\n\n";

    cmakeContent += getPlatformCMakeConfig() + "\n";

    cmakeContent += scriptSources + "\n";
    cmakeContent += factorySources + "\n";
    cmakeContent += "set(PROJECT_SOURCE " + internalPathStr + "/init_scripts.cpp)\n\n";
    cmakeContent += "# Project target\n";
    cmakeContent += "if(NOT CMAKE_SYSTEM_NAME STREQUAL \"Android\" AND NOT SUPERNOVA_EDITOR_PLUGIN)\n";
    cmakeContent += "    add_executable(" + libName + "\n";
    cmakeContent += "        ${PROJECT_SOURCE}\n";
    cmakeContent += "        ${SCRIPT_SOURCES}\n";
    cmakeContent += "        ${PLATFORM_SOURCE}\n";
    cmakeContent += "    )\n";
    cmakeContent += "else()\n";
    cmakeContent += "    add_library(" + libName + " SHARED\n";
    cmakeContent += "        ${PROJECT_SOURCE}\n";
    cmakeContent += "        ${SCRIPT_SOURCES}\n";
    cmakeContent += "        ${PLATFORM_SOURCE}\n";
    cmakeContent += "    )\n";
    cmakeContent += "endif()\n\n";

    cmakeContent += "# When building outside the editor, also compile Factory-generated sources\n";
    cmakeContent += "if(NOT SUPERNOVA_EDITOR_PLUGIN)\n";
    cmakeContent += "    target_sources(" + libName + " PRIVATE ${FACTORY_SOURCES})\n";
    cmakeContent += "endif()\n\n";
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

    cmakeContent += "# Default SUPERNOVA_LIB_DIR if not provided\n";
    cmakeContent += "if(NOT DEFINED SUPERNOVA_LIB_DIR OR SUPERNOVA_LIB_DIR STREQUAL \"\")\n";
    cmakeContent += "    set(SUPERNOVA_LIB_DIR \"" + exePath.generic_string() + "\")\n";
    cmakeContent += "endif()\n\n";

    cmakeContent += "# Find supernova library in specified location\n";
    cmakeContent += "find_library(SUPERNOVA_LIB supernova PATHS ${SUPERNOVA_LIB_DIR} NO_DEFAULT_PATH)\n";
    cmakeContent += "if(NOT SUPERNOVA_LIB)\n";
    cmakeContent += "    message(FATAL_ERROR \"Supernova library not found in ${SUPERNOVA_LIB_DIR}\")\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "target_link_libraries(" + libName + " PRIVATE ${SUPERNOVA_LIB} ${PLATFORM_LIBS})\n\n";
    cmakeContent += "# Set compile options based on compiler and platform\n";
    cmakeContent += "if(MSVC)\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE /W4 /EHsc)\n";
    cmakeContent += "else()\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE -Wall -Wextra -fPIC)\n";
    cmakeContent += "    target_link_options(" + libName + " PRIVATE -Wl,-z,defs,--no-undefined)\n"; // no undefined symbols
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
    sourceContent += "// This file is auto-generated by Supernova Editor. Do not edit manually.\n\n";
    sourceContent += "#include <vector>\n";
    sourceContent += "#include <string>\n";
    sourceContent += "#include <stdio.h>\n";
    sourceContent += "#include \"Supernova.h\"\n\n";

    for (const auto& s : scriptFiles) {
        fs::path headerPath = s.headerPath;
        if (headerPath.is_relative()) {
            headerPath = projectPath / headerPath;
        }
        if (!headerPath.empty() && fs::exists(headerPath) && fs::is_regular_file(headerPath)) {
            fs::path relativePath = fs::relative(headerPath, projectPath);
            std::string inc = relativePath.generic_string();
            sourceContent += "#include \"" + inc + "\"\n";
        }
    }

    sourceContent += "\n";
    sourceContent += "using namespace Supernova;\n";

    sourceContent += "\n";
    sourceContent += "#if defined(_MSC_VER)\n";
    sourceContent += "    #define PROJECT_API __declspec(dllexport)\n";
    sourceContent += "#else\n";
    sourceContent += "    #define PROJECT_API\n";
    sourceContent += "#endif\n\n";

    sourceContent += "extern \"C\" void PROJECT_API initScene(Supernova::Scene* scene) {\n";

    if (!scriptFiles.empty()) {

        sourceContent += "\n";

        sourceContent += "    const auto& scriptsArray = scene->getComponentArray<ScriptComponent>();\n";
        sourceContent += "\n";

        sourceContent += "    for (size_t i = 0; i < scriptsArray->size(); i++) {\n";
        sourceContent += "        Supernova::ScriptComponent& scriptComp = scriptsArray->getComponentFromIndex(i);\n";
        sourceContent += "        Supernova::Entity entity = scriptsArray->getEntity(i);\n";
        sourceContent += "        for (auto& scriptEntry : scriptComp.scripts) {\n";
        sourceContent += "            if (scriptEntry.type == ScriptType::SCRIPT_LUA) \n";
        sourceContent += "                continue; \n";
        sourceContent += "\n";

        for (const auto& s : scriptFiles) {
            sourceContent += "            if (scriptEntry.className == \"" + s.className + "\") {\n";
            sourceContent += "                " + s.className + "* script = new " + s.className + "(scene, entity);\n";
            sourceContent += "                scriptEntry.instance = static_cast<void*>(script);\n";
            sourceContent += "            }\n";
        }
        sourceContent += "        }\n";
        sourceContent += "    }\n";

        sourceContent += "    for (size_t i = 0; i < scriptsArray->size(); i++) {\n";
        sourceContent += "        Supernova::ScriptComponent& scriptComp = scriptsArray->getComponentFromIndex(i);\n";
        sourceContent += "        for (auto& scriptEntry : scriptComp.scripts) {\n";
        sourceContent += "            if (scriptEntry.type == ScriptType::SCRIPT_LUA) \n";
        sourceContent += "                continue; \n";
        sourceContent += "\n";

        for (const auto& s : scriptFiles) {
            sourceContent += "            if (scriptEntry.className == \"" + s.className + "\") {\n";

            if (s.scene->findComponent<ScriptComponent>(s.entity)) {
                ScriptComponent* sc = s.scene->findComponent<ScriptComponent>(s.entity);
                for (const auto& entry : sc->scripts) {
                    if (entry.path == s.path && entry.headerPath == s.headerPath && !entry.properties.empty()) {
                        sourceContent += "                " + s.className + "* typedScript = static_cast<" + s.className + "*>(scriptEntry.instance);\n";
                        sourceContent += "\n";
                        sourceContent += "                for (auto& prop : scriptEntry.properties) {\n";

                        // Generate individual property assignments
                        for (const auto& prop : entry.properties) {
                            sourceContent += "\n";
                            sourceContent += "                    if (prop.name == \"" + prop.name + "\") {\n";

                            // Handle EntityPointer types
                            if (prop.type == ScriptPropertyType::EntityPointer) {
                                sourceContent += "                        Supernova::EntityRef entityRef = std::get<Supernova::EntityRef>(prop.value);\n";
                                sourceContent += "                        void* instancePtr = nullptr;\n";
                                sourceContent += "\n";
                                sourceContent += "                        if (entityRef.entity != NULL_ENTITY && entityRef.scene) {\n";
                                sourceContent += "                            Supernova::ScriptComponent* targetScriptComp = entityRef.scene->findComponent<Supernova::ScriptComponent>(entityRef.entity);\n";
                                sourceContent += "                            if (targetScriptComp) {\n";
                                sourceContent += "                                for (auto& targetScript : targetScriptComp->scripts) {\n";
                                sourceContent += "                                    if (targetScript.type != ScriptType::SCRIPT_LUA) {\n";
                                sourceContent += "                                        if (targetScript.className == \"" + prop.ptrTypeName + "\" && targetScript.instance) {\n";
                                sourceContent += "                                            instancePtr = targetScript.instance;\n";
                                sourceContent += "                                            printf(\"[DEBUG]   Found matching C++ script instance: '%s'\\n\", targetScript.className.c_str());\n";
                                sourceContent += "                                            break;\n";
                                sourceContent += "                                        }\n";
                                sourceContent += "                                    }\n";
                                sourceContent += "                                }\n";
                                sourceContent += "                            }\n";
                                sourceContent += "\n";
                                if (!prop.ptrTypeName.empty()) {
                                    sourceContent += "                            if (!instancePtr) {\n";
                                    sourceContent += "                                printf(\"[DEBUG]   No C++ script instance found, creating '" + prop.ptrTypeName + "' type\\n\");\n";
                                    sourceContent += "                                instancePtr = new " + prop.ptrTypeName + "(entityRef.scene, entityRef.entity);\n";
                                    sourceContent += "                            }\n";
                                }
                                sourceContent += "                        }\n";
                                sourceContent += "\n";
                                sourceContent += "                        typedScript->" + prop.name + " = nullptr;\n";
                                sourceContent += "                        if (instancePtr) {\n";
                                sourceContent += "                            typedScript->" + prop.name + " = static_cast<" + prop.ptrTypeName + "*>(instancePtr);\n";
                                sourceContent += "                        }\n";
                                sourceContent += "\n";
                            }

                            sourceContent += "                        prop.memberPtr = &typedScript->" + prop.name + ";\n";
                            sourceContent += "                    }\n";
                        }

                        sourceContent += "\n";
                        sourceContent += "                    prop.syncToMember();\n";
                        sourceContent += "                }\n";
                    }
                }
            }

            sourceContent += "            }\n";
        }

        sourceContent += "\n";
        sourceContent += "        }\n";
        sourceContent += "    }\n";

    } else{
        sourceContent += "    (void)scene; // Suppress unused parameter warning\n";
    }

    sourceContent += "}\n\n";

    // Cleanup function - delete all script instances
    sourceContent += "extern \"C\" void PROJECT_API cleanup(Supernova::Scene* scene) {\n";

    if (!scriptFiles.empty()) {
        sourceContent += "    const auto& scriptsArray = scene->getComponentArray<ScriptComponent>();\n";
        sourceContent += "    for (size_t i = 0; i < scriptsArray->size(); i++) {\n";
        sourceContent += "        Supernova::ScriptComponent& scriptComp = scriptsArray->getComponentFromIndex(i);\n";
        sourceContent += "        for (auto& scriptEntry : scriptComp.scripts) {\n";
        sourceContent += "            if (scriptEntry.type == ScriptType::SCRIPT_LUA) \n";
        sourceContent += "                continue; \n";
        sourceContent += "\n";
        for (const auto& s : scriptFiles) {
            sourceContent += "            if (scriptEntry.instance) {\n";
            sourceContent += "                if (scriptEntry.className == \"" + s.className + "\") {\n";
            sourceContent += "                    delete static_cast<" + s.className + "*>(scriptEntry.instance);\n";
            sourceContent += "                }\n";
            sourceContent += "                scriptEntry.instance = nullptr;\n";
            sourceContent += "            }\n";
        }
        sourceContent += "        }\n";
        sourceContent += "\n";
        sourceContent += "        Engine::clearAllSubscriptions(true);\n";
        sourceContent += "    }\n";
    } else {
        sourceContent += "    (void)scene; // Suppress unused parameter warning\n";
    }

    sourceContent += "}\n";

    const fs::path cmakeFile = projectPath / "CMakeLists.txt";
    const fs::path sourceFile = projectInternalPath / "init_scripts.cpp";

    writeIfChanged(cmakeFile, cmakeContent);
    writeIfChanged(sourceFile, sourceContent);
}

void Editor::Generator::terminateCurrentProcess() {
    #ifdef _WIN32
        HANDLE handleSnapshot = nullptr;
        {
            std::lock_guard<std::mutex> lock(processHandleMutex);
            handleSnapshot = currentProcessHandle;
        }
        if (handleSnapshot) {
            TerminateProcess(handleSnapshot, 1);
        }
    #else
        pid_t pidSnapshot = 0;
        {
            std::lock_guard<std::mutex> lock(processPidMutex);
            pidSnapshot = currentProcessPid;
        }
        if (pidSnapshot > 0) {
            if (kill(pidSnapshot, SIGTERM) != 0 && errno != ESRCH) {
                Out::warning("Failed to send SIGTERM to pid %d: %s", pidSnapshot, strerror(errno));
            }

            std::this_thread::sleep_for(kKillGracePeriod);
            if (kill(pidSnapshot, 0) == 0) {
                kill(pidSnapshot, SIGKILL);
            }
        }
    #endif
}

void Editor::Generator::configure(const std::vector<SceneData>& scenes, const fs::path& projectInternalPath){
    const fs::path generatedPath = projectInternalPath / "generated";

    if (generatedPath.empty() || generatedPath == generatedPath.root_path()) {
        Out::error("Refusing to clear generated directory: invalid path '%s'", generatedPath.string().c_str());
        return;
    }

    std::error_code ec;
    if (fs::exists(generatedPath, ec)) {
        fs::remove_all(generatedPath, ec);
        if (ec) {
            Out::warning("Failed to clear generated directory '%s': %s", generatedPath.string().c_str(), ec.message().c_str());
        }
    }

    ec.clear();
    fs::create_directories(generatedPath, ec);
    if (ec) {
        Out::error("Failed to create generated directory '%s': %s", generatedPath.string().c_str(), ec.message().c_str());
        return;
    }

    for (const auto& sceneData : scenes) {
        std::string sceneContent = Factory::createScene(0, sceneData.scene, sceneData.name, sceneData.entities, sceneData.camera);

        std::string filename = Factory::toIdentifier(sceneData.name) + ".cpp";
        const fs::path sourceFile = generatedPath / filename;

        writeIfChanged(sourceFile, sceneContent);
    }

    // Build init.cpp content
    std::string initContent;
    initContent += "#include \"Supernova.h\"\n";
    initContent += "\n";
    initContent += "using namespace Supernova;\n\n";

    // initScene is generated in init_scripts.cpp; call it after initializing each scene
    initContent += "extern \"C\" void initScene(Supernova::Scene* scene);\n\n";

    // Forward declarations for per-scene initialization functions (defined in generated scene .cpp files)
    for (const auto& sceneData : scenes) {
        std::string sceneName = Factory::toIdentifier(sceneData.name);
        initContent += "void create_" + sceneName + "(Scene* scene);\n";
    }
    initContent += "\n";

    for (const auto& sceneData : scenes) {
        std::string sceneName = Factory::toIdentifier(sceneData.name);
        initContent += "Scene " + sceneName + ";\n";
    }
    initContent += "\n";

    initContent += "SUPERNOVA_INIT void init() {\n";
    for (const auto& sceneData : scenes) {
        std::string sceneName = Factory::toIdentifier(sceneData.name);
        initContent += "    create_" + sceneName + "(&" + sceneName + ");\n";
    }

    for (const auto& sceneData : scenes) {
        std::string sceneName = Factory::toIdentifier(sceneData.name);
        initContent += "    initScene(&" + sceneName + ");\n";
    }
    initContent += "\n";
    initContent += "    Engine::setCanvasSize(1000, 480);\n";
    for (const auto& sceneData : scenes) {
        std::string sceneName = Factory::toIdentifier(sceneData.name);
        if (sceneData.isMain) {
            initContent += "    Engine::setScene(&" + sceneName + ");\n";
        } else {
            initContent += "    Engine::addSceneLayer(&" + sceneName + ");\n";
        }
    }
    initContent += "}\n";

    const fs::path initFile = generatedPath / "init.cpp";
    writeIfChanged(initFile, initContent);

    // Build main.cpp content
    std::string mainContent;
    mainContent += "#include \"PlatformEditor.h\"\n";
    mainContent += "\n";

    mainContent += "int main(int argc, char* argv[]) {\n";
    mainContent += "    return PlatformEditor::init(argc, argv);\n";
    mainContent += "}\n";

    const fs::path mainFile = generatedPath / "main.cpp";
    writeIfChanged(mainFile, mainContent);

    const fs::path platformHeaderFile = generatedPath / "PlatformEditor.h";
    writeIfChanged(platformHeaderFile, getPlatformEditorHeader());

    const fs::path platformSourceFile = generatedPath / "PlatformEditor.cpp";
    writeIfChanged(platformSourceFile, getPlatformEditorSource());
}

std::string Editor::Generator::getPlatformEditorHeader() {
    std::string content;
    content += "#pragma once\n\n";
    content += "#define GLFW_INCLUDE_NONE\n";
    content += "#include \"GLFW/glfw3.h\"\n\n";
    content += "#include \"System.h\"\n\n";
    content += "class PlatformEditor: public Supernova::System{\n\n";
    content += "private:\n\n";
    content += "    static int windowPosX;\n";
    content += "    static int windowPosY;\n";
    content += "    static int windowWidth;\n";
    content += "    static int windowHeight;\n\n";
    content += "    static int screenWidth;\n";
    content += "    static int screenHeight;\n\n";
    content += "    static double mousePosX;\n";
    content += "    static double mousePosY;\n\n";
    content += "    static int sampleCount;\n\n";
    content += "    static GLFWwindow* window;\n";
    content += "    static GLFWmonitor* monitor;\n\n";
    content += "public:\n\n";
    content += "    PlatformEditor();\n\n";
    content += "    static int init(int argc, char **argv);\n\n";
    content += "    virtual int getScreenWidth();\n";
    content += "    virtual int getScreenHeight();\n\n";
    content += "    virtual int getSampleCount();\n\n";
    content += "    virtual bool isFullscreen();\n";
    content += "    virtual void requestFullscreen();\n";
    content += "    virtual void exitFullscreen();\n\n";
    content += "    virtual void setMouseCursor(Supernova::CursorType type);\n";
    content += "    virtual void setShowCursor(bool showCursor);\n\n";
    content += "    virtual std::string getAssetPath();\n";
    content += "    virtual std::string getUserDataPath();\n";
    content += "    virtual std::string getLuaPath();\n\n";
    content += "};\n";
    return content;
}

std::string Editor::Generator::getPlatformEditorSource() {
    std::string content;
    content += "#include \"PlatformEditor.h\"\n\n";
    content += "#include \"Engine.h\"\n\n";
    content += "int PlatformEditor::windowPosX;\n";
    content += "int PlatformEditor::windowPosY;\n";
    content += "int PlatformEditor::windowWidth;\n";
    content += "int PlatformEditor::windowHeight;\n\n";
    content += "int PlatformEditor::screenWidth;\n";
    content += "int PlatformEditor::screenHeight;\n\n";
    content += "double PlatformEditor::mousePosX;\n";
    content += "double PlatformEditor::mousePosY;\n\n";
    content += "int PlatformEditor::sampleCount;\n\n";
    content += "GLFWwindow* PlatformEditor::window;\n";
    content += "GLFWmonitor* PlatformEditor::monitor;\n\n\n";
    content += "PlatformEditor::PlatformEditor(){\n\n";
    content += "}\n\n";
    content += "int PlatformEditor::init(int argc, char **argv){\n";
    content += "    windowWidth = DEFAULT_WINDOW_WIDTH;\n";
    content += "    windowHeight = DEFAULT_WINDOW_HEIGHT;\n\n";
    content += "    sampleCount = 1;\n\n";
    content += "    Supernova::Engine::systemInit(argc, argv, new PlatformEditor());\n\n";
    content += "    /* create window and GL context via GLFW */\n";
    content += "    glfwInit();\n";
    content += "    glfwWindowHint(GLFW_SAMPLES, (sampleCount == 1) ? 0 : sampleCount);\n";
    content += "    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);\n";
    content += "    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);\n";
    content += "    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);\n";
    content += "    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);\n";
    content += "    window = glfwCreateWindow(windowWidth, windowHeight, \"Supernova\", 0, 0);\n\n";
    content += "    glfwMakeContextCurrent(window);\n";
    content += "    glfwSwapInterval(1);\n\n";
    content += "    monitor = glfwGetPrimaryMonitor();\n\n";
    content += "    glfwSetMouseButtonCallback(window, [](GLFWwindow*, int btn, int action, int mods) {\n";
    content += "        if (action==GLFW_PRESS){\n";
    content += "            Supernova::Engine::systemMouseDown(btn, float(mousePosX), float(mousePosY), mods);\n";
    content += "        }else if (action==GLFW_RELEASE){\n";
    content += "            Supernova::Engine::systemMouseUp(btn, float(mousePosX), float(mousePosY), mods);\n";
    content += "        }\n";
    content += "    });\n";
    content += "    glfwSetCursorPosCallback(window, [](GLFWwindow*, double pos_x, double pos_y) {\n";
    content += "        float xscale, yscale;\n";
    content += "        glfwGetWindowContentScale(window, &xscale, &yscale);\n\n";
    content += "        mousePosX = pos_x * xscale;\n";
    content += "        mousePosY = pos_y * yscale;\n";
    content += "        Supernova::Engine::systemMouseMove(float(pos_x), float(pos_y), 0);\n";
    content += "    });\n";
    content += "    glfwSetScrollCallback(window, [](GLFWwindow*, double pos_x, double pos_y){\n";
    content += "        Supernova::Engine::systemMouseScroll((float)pos_x, (float)pos_y, 0);\n";
    content += "    });\n";
    content += "    glfwSetKeyCallback(window, [](GLFWwindow*, int key, int /*scancode*/, int action, int mods){\n";
    content += "        if (action==GLFW_PRESS){\n";
    content += "            if (key == GLFW_KEY_TAB)\n";
    content += "                Supernova::Engine::systemCharInput('\\t');\n";
    content += "            if (key == GLFW_KEY_BACKSPACE)\n";
    content += "                Supernova::Engine::systemCharInput('\\b');\n";
    content += "            if (key == GLFW_KEY_ENTER)\n";
    content += "                Supernova::Engine::systemCharInput('\\r');\n";
    content += "            if (key == GLFW_KEY_ESCAPE)\n";
    content += "                Supernova::Engine::systemCharInput('\\e');\n";
    content += "            Supernova::Engine::systemKeyDown(key, false, mods);\n";
    content += "        }else if (action==GLFW_REPEAT){\n";
    content += "            Supernova::Engine::systemKeyDown(key, true, mods);\n";
    content += "        }else if (action==GLFW_RELEASE){\n";
    content += "            Supernova::Engine::systemKeyUp(key, false, mods);\n";
    content += "        }\n";
    content += "    });\n";
    content += "    glfwSetCharCallback(window, [](GLFWwindow*, unsigned int codepoint){\n";
    content += "        Supernova::Engine::systemCharInput(codepoint);\n";
    content += "    });\n\n";
    content += "    int cur_width, cur_height;\n";
    content += "    glfwGetFramebufferSize(window, &cur_width, &cur_height);\n\n";
    content += "    PlatformEditor::screenWidth = cur_width;\n";
    content += "    PlatformEditor::screenHeight = cur_height;\n\n";
    content += "    Supernova::Engine::systemViewLoaded();\n";
    content += "    Supernova::Engine::systemViewChanged();\n\n";
    content += "    /* draw loop */\n";
    content += "    while (!glfwWindowShouldClose(window)) {\n";
    content += "        int cur_width, cur_height;\n";
    content += "        glfwGetFramebufferSize(window, &cur_width, &cur_height);\n\n";
    content += "        if (cur_width != PlatformEditor::screenWidth || cur_height != PlatformEditor::screenHeight){\n";
    content += "            PlatformEditor::screenWidth = cur_width;\n";
    content += "            PlatformEditor::screenHeight = cur_height;\n";
    content += "            Supernova::Engine::systemViewChanged();\n";
    content += "        }\n\n";
    content += "        Supernova::Engine::systemDraw();\n\n";
    content += "        glfwSwapBuffers(window);\n";
    content += "        glfwPollEvents();\n";
    content += "    }\n\n";
    content += "    Supernova::Engine::systemViewDestroyed();\n";
    content += "    Supernova::Engine::systemShutdown();\n";
    content += "    glfwTerminate();\n";
    content += "    return 0;\n";
    content += "}\n\n";
    content += "int PlatformEditor::getScreenWidth(){\n";
    content += "    return PlatformEditor::screenWidth;\n";
    content += "}\n\n";
    content += "int PlatformEditor::getScreenHeight(){\n";
    content += "    return PlatformEditor::screenHeight;\n";
    content += "}\n\n";
    content += "int PlatformEditor::getSampleCount(){\n";
    content += "    return PlatformEditor::sampleCount;\n";
    content += "}\n\n";
    content += "bool PlatformEditor::isFullscreen(){\n";
    content += "    return glfwGetWindowMonitor(window) != nullptr;\n";
    content += "}\n\n";
    content += "void PlatformEditor::requestFullscreen(){\n";
    content += "    if (isFullscreen())\n";
    content += "        return;\n\n";
    content += "    // backup window position and window size\n";
    content += "    glfwGetWindowPos(window, &windowPosX, &windowPosY);\n";
    content += "    glfwGetWindowSize(window, &windowWidth, &windowHeight);\n\n";
    content += "    // get resolution of monitor\n";
    content += "    const GLFWvidmode * mode = glfwGetVideoMode(monitor);\n\n";
    content += "    // switch to full screen\n";
    content += "    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0 );\n";
    content += "}\n\n";
    content += "void PlatformEditor::exitFullscreen(){\n";
    content += "    if (!isFullscreen())\n";
    content += "        return;\n\n";
    content += "    // restore last window size and position\n";
    content += "    glfwSetWindowMonitor(window, nullptr,  windowPosX, windowPosY, windowWidth, windowHeight, 0);\n";
    content += "}\n\n";
    content += "void PlatformEditor::setMouseCursor(Supernova::CursorType type){\n";
    content += "    GLFWcursor* cursor = NULL;\n\n";
    content += "    if (type == Supernova::CursorType::ARROW){\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);\n";
    content += "    }else if (type == Supernova::CursorType::IBEAM){\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);\n";
    content += "    }else if (type == Supernova::CursorType::CROSSHAIR){\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);\n";
    content += "    }else if (type == Supernova::CursorType::POINTING_HAND){\n";
    content += "        #ifdef GLFW_POINTING_HAND_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::RESIZE_EW){\n";
    content += "        #ifdef GLFW_RESIZE_EW_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::RESIZE_NS){\n";
    content += "        #ifdef GLFW_RESIZE_NS_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::RESIZE_NWSE){\n";
    content += "        #ifdef GLFW_RESIZE_NWSE_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::RESIZE_NESW){\n";
    content += "        #ifdef GLFW_RESIZE_NESW_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::RESIZE_ALL){\n";
    content += "        #ifdef GLFW_RESIZE_ALL_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);\n";
    content += "        #endif\n";
    content += "    }else if (type == Supernova::CursorType::NOT_ALLOWED){\n";
    content += "        #ifdef GLFW_NOT_ALLOWED_CURSOR\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);\n";
    content += "        #else\n";
    content += "        cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);\n";
    content += "        #endif\n";
    content += "    }\n\n";
    content += "    if (cursor) {\n";
    content += "        glfwSetCursor(window, cursor);\n";
    content += "    } else {\n";
    content += "        // Handle error: cursor creation failed\n";
    content += "    }\n";
    content += "}\n\n";
    content += "void PlatformEditor::setShowCursor(bool showCursor){\n";
    content += "    if (showCursor){\n";
    content += "        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);\n";
    content += "    }else{\n";
    content += "        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);\n";
    content += "    }\n";
    content += "}\n\n";
    content += "std::string PlatformEditor::getAssetPath(){\n";
    content += "    return \"assets\";\n";
    content += "}\n\n";
    content += "std::string PlatformEditor::getUserDataPath(){\n";
    content += "    return \".\";\n";
    content += "}\n\n";
    content += "std::string PlatformEditor::getLuaPath(){\n";
    content += "    return \"lua\";\n";
    content += "}\n";
    return content;
}

void Editor::Generator::build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath, std::string libName, const std::vector<ScriptSource>& scriptFiles, const std::vector<SceneData>& scenes) {
    writeSourceFiles(projectPath, projectInternalPath, libName, scriptFiles, scenes);

    cancelBuild();
    waitForBuildToComplete();

    lastBuildSucceeded.store(false, std::memory_order_relaxed);
    cancelRequested.store(false, std::memory_order_relaxed);

    buildFuture = std::async(std::launch::async, [this, projectPath, buildPath]() {
        try {
            auto startTime = std::chrono::steady_clock::now();

            std::string configType = "Debug";

            if (!configureCMake(projectPath, buildPath, configType)) {
                if (cancelRequested.load(std::memory_order_relaxed)) {
                    Out::warning("Build configuration cancelled.");
                } else {
                    Out::error("CMake configuration failed");
                }
                lastBuildSucceeded.store(false, std::memory_order_relaxed);
                return;
            }

            if (!buildProject(projectPath, buildPath, configType)) {
                if (cancelRequested.load(std::memory_order_relaxed)) {
                    Out::warning("Build cancelled.");
                } else {
                    Out::error("Build failed");
                }
                lastBuildSucceeded.store(false, std::memory_order_relaxed);
                return;
            }

            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            double seconds = duration.count() / 1000.0;
            Out::build("Build completed successfully in %.2f seconds.", seconds);
            lastBuildSucceeded.store(true, std::memory_order_relaxed);
        } catch (const std::exception& ex) {
            Out::error("Build exception: %s", ex.what());
            lastBuildSucceeded.store(false, std::memory_order_relaxed);
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
    return lastBuildSucceeded.load(std::memory_order_relaxed);
}

std::future<void> Editor::Generator::cancelBuild() {
    // Launch cancellation on a separate thread so the UI thread is not blocked.
    return std::async(std::launch::async, [this]() {
        // Check if build is in progress inside the async task
        if (!isBuildInProgress()) {
            cancelRequested.store(false, std::memory_order_relaxed);
            return;
        }

        Out::warning("Cancelling build process...");
        cancelRequested.store(true, std::memory_order_relaxed);

        // Attempt to terminate the running process, then wait for the build to complete.
        terminateCurrentProcess();
        waitForBuildToComplete();
        cancelRequested.store(false, std::memory_order_relaxed);
        lastBuildSucceeded.store(false, std::memory_order_relaxed);
        Out::warning("Build process cancelled.");
    });
}