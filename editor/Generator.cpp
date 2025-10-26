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
    std::filesystem::path exePath;
    #ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        exePath = std::filesystem::path(path).parent_path();
    #else
        exePath = std::filesystem::canonical("/proc/self/exe").parent_path();
    #endif

    std::string cmakeCommand = "cmake ";
    #ifdef _WIN32
        cmakeCommand += "-G \"Visual Studio 17 2022\" ";
    #endif
    cmakeCommand += "-DCMAKE_BUILD_TYPE=" + configType + " ";
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
        std::ofstream ofs(filePath, std::ios::out | std::ios::binary);
        ofs << newContent;
        return true;
    }
    return false;
}

void Editor::Generator::writeSourceFiles(const fs::path& projectPath, const fs::path& projectInternalPath, std::string libName, const std::vector<ScriptSource>& scriptFiles) {
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
    sourceContent += "#include <stdio.h>\n";
    sourceContent += "#include \"Supernova.h\"\n\n";

    for (const auto& s : scriptFiles) {
        if (!s.headerPath.empty() && fs::exists(s.headerPath)) {
            fs::path relativePath = fs::relative(s.headerPath, projectPath);
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

    sourceContent += "\n";

    sourceContent += "    const auto& scriptsArray = scene->getComponentArray<ScriptComponent>();\n";
    sourceContent += "    for (size_t i = 0; i < scriptsArray->size(); i++) {\n";
    sourceContent += "        Supernova::ScriptComponent& scriptComp = scriptsArray->getComponentFromIndex(i);\n";
    sourceContent += "        Supernova::Entity entity = scriptsArray->getEntity(i);\n";
    sourceContent += "        for (auto& scriptEntry : scriptComp.scripts) {\n";
    sourceContent += "\n";

    for (const auto& s : scriptFiles) {
        sourceContent += "            if (scriptEntry.className == \"" + s.className + "\") {\n";
        sourceContent += "                " + s.className + "* script = new " + s.className + "(scene, entity);\n";
        sourceContent += "                scriptEntry.instance = static_cast<void*>(script);\n";

        if (s.scene->findComponent<ScriptComponent>(s.entity)) {
            ScriptComponent* sc = s.scene->findComponent<ScriptComponent>(s.entity);
            for (const auto& entry : sc->scripts) {
                if (entry.className == s.className && !entry.properties.empty()) {
                    sourceContent += "\n";
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
                            sourceContent += "                        printf(\"[DEBUG]   EntityRef: entity=%u\\n\", (unsigned int)entityRef.entity);\n";
                            sourceContent += "                        void* instancePtr = nullptr;\n";
                            sourceContent += "\n";
                            sourceContent += "                        if (entityRef.entity != NULL_ENTITY && entityRef.scene) {\n";
                            sourceContent += "                            Supernova::ScriptComponent* targetScriptComp = entityRef.scene->findComponent<Supernova::ScriptComponent>(entityRef.entity);\n";
                            sourceContent += "                            if (targetScriptComp) {\n";
                            sourceContent += "                                for (auto& targetScript : targetScriptComp->scripts) {\n";
                            sourceContent += "                                    if (targetScript.type == ScriptType::SUBCLASS && targetScript.className == \"" + prop.ptrTypeName + "\" && targetScript.instance) {\n";
                            sourceContent += "                                        instancePtr = targetScript.instance;\n";
                            sourceContent += "                                        printf(\"[DEBUG]   Found matching script instance: %p\\n\", instancePtr);\n";
                            sourceContent += "                                        break;\n";
                            sourceContent += "                                    }\n";
                            sourceContent += "                                }\n";
                            sourceContent += "                            }\n";
                            sourceContent += "\n";
                            if (!prop.ptrTypeName.empty()) {
                                sourceContent += "                            if (!instancePtr) {\n";
                                sourceContent += "                                printf(\"[DEBUG]   No script instance found, creating " + prop.ptrTypeName + " type\\n\");\n";
                                sourceContent += "                                instancePtr = new " + prop.ptrTypeName + "(entityRef.scene, entityRef.entity);\n";
                                sourceContent += "                            }\n";
                            }
                            sourceContent += "                        }\n";
                            sourceContent += "\n";
                            sourceContent += "                        typedScript->" + prop.name + " = nullptr;\n";
                            sourceContent += "                        if (instancePtr) {\n";
                            sourceContent += "                            typedScript->" + prop.name + " = static_cast<" + prop.ptrTypeName + "*>(instancePtr);\n";
                            sourceContent += "                            printf(\"[DEBUG]   Successfully assigned %p to typedScript->" + prop.name + "\\n\", instancePtr);\n";
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

    if (scriptFiles.empty()) {
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
        for (const auto& s : scriptFiles) {
            sourceContent += "            if (scriptEntry.instance) {\n";
            sourceContent += "                if (scriptEntry.className == \"" + s.className + "\") {\n";
            sourceContent += "                    delete static_cast<" + s.className + "*>(scriptEntry.instance);\n";
            sourceContent += "                }\n";
            sourceContent += "                scriptEntry.instance = nullptr;\n";
            sourceContent += "            }\n";
        }
        sourceContent += "        }\n";
        sourceContent += "    }\n";
    } else {
        sourceContent += "    (void)scene; // Suppress unused parameter warning\n";
    }

    sourceContent += "}\n";

    const fs::path cmakeFile = projectPath / "CMakeLists.txt";
    const fs::path sourceFile = projectInternalPath / "project_main.cpp";

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

void Editor::Generator::build(const fs::path projectPath, const fs::path projectInternalPath, const fs::path buildPath, std::string libName, const std::vector<ScriptSource>& scriptFiles) {
    writeSourceFiles(projectPath, projectInternalPath, libName, scriptFiles);

    cancelBuild();
    waitForBuildToComplete();

    lastBuildSucceeded.store(false, std::memory_order_relaxed);
    cancelRequested.store(false, std::memory_order_relaxed);

    buildFuture = std::async(std::launch::async, [this, projectPath, buildPath]() {
        try {
            auto startTime = std::chrono::steady_clock::now();

            #ifdef _DEBUG
                std::string configType = "Debug";
            #else
                std::string configType = "Release";
            #endif
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