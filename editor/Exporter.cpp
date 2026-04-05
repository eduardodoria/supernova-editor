#include "Exporter.h"
#include "App.h"
#include "Out.h"
#include "util/FileUtils.h"
#include "pool/ShaderPool.h"

#include <fstream>

using namespace Supernova;

Editor::Exporter::Exporter() {
}

Editor::Exporter::~Exporter() {
    if (exportThread.joinable()) {
        exportThread.join();
    }
}

void Editor::Exporter::setProgress(const std::string& step, float value) {
    std::lock_guard<std::mutex> lock(progressMutex);
    progress.currentStep = step;
    progress.overallProgress = value;
}

void Editor::Exporter::setError(const std::string& message) {
    std::lock_guard<std::mutex> lock(progressMutex);
    progress.failed = true;
    progress.errorMessage = message;
    Out::error("Export failed: %s", message.c_str());
}

Editor::ExportProgress Editor::Exporter::getProgress() const {
    std::lock_guard<std::mutex> lock(progressMutex);
    return progress;
}

bool Editor::Exporter::isRunning() const {
    std::lock_guard<std::mutex> lock(progressMutex);
    return !progress.finished && !progress.failed;
}

void Editor::Exporter::startExport(Project* proj, const ExportConfig& cfg) {
    if (exportThread.joinable()) {
        exportThread.join();
    }

    this->project = proj;
    this->config = cfg;
    {
        std::lock_guard<std::mutex> lock(progressMutex);
        this->progress = ExportProgress();
    }

    exportThread = std::thread(&Exporter::runExport, this);
}

void Editor::Exporter::runExport() {

    if (!checkTargetDir()) return;
    if (!cleanGenerated()) return;
    if (!saveAllScenes()) return;
    if (!copyGenerated()) return;
    if (!copyAssets()) return;
    if (!copyCppScripts()) return;
    if (!copyEngine()) return;
    if (!buildAndSaveShaders()) return;
    if (!generateCMakeLists()) return;

    setProgress("Export complete", 1.0f);
    {
        std::lock_guard<std::mutex> lock(progressMutex);
        progress.finished = true;
    }
    Out::info("Project exported successfully to: %s", config.targetDir.string().c_str());
}

bool Editor::Exporter::checkTargetDir() {
    setProgress("Checking target directory...", 0.0f);

    if (config.targetDir.empty()) {
        setError("Target directory not specified");
        return false;
    }

    std::error_code ec;
    if (!fs::exists(config.targetDir, ec)) {
        fs::create_directories(config.targetDir, ec);
        if (ec) {
            setError("Failed to create target directory: " + ec.message());
            return false;
        }
    }

    if (!fs::is_empty(config.targetDir, ec)) {
        setError("Target directory is not empty");
        return false;
    }

    return true;
}

bool Editor::Exporter::cleanGenerated() {
    setProgress("Cleaning generated directory...", 0.05f);

    fs::path generatedDir = project->getProjectInternalPath() / "generated";

    std::error_code ec;
    if (fs::exists(generatedDir, ec)) {
        fs::remove_all(generatedDir, ec);
        if (ec) {
            setError("Failed to clean generated directory: " + ec.message());
            return false;
        }
    }
    fs::create_directories(generatedDir, ec);
    if (ec) {
        setError("Failed to recreate generated directory: " + ec.message());
        return false;
    }

    return true;
}

bool Editor::Exporter::saveAllScenes() {
    setProgress("Saving all scenes...", 0.1f);

    for (auto& sceneProject : project->getScenes()) {
        if (!sceneProject.scene) {
            continue; // scene not loaded, already saved on disk
        }
        fs::path fullPath = sceneProject.filepath;
        if (fullPath.is_relative()) {
            fullPath = project->getProjectPath() / fullPath;
        }
        project->saveSceneToPath(sceneProject.id, fullPath);
    }

    return true;
}

bool Editor::Exporter::copyGenerated() {
    setProgress("Copying generated files...", 0.2f);

    fs::path generatedSrc = project->getProjectInternalPath() / "generated";
    fs::path generatedDst = config.targetDir / "generated";

    std::error_code ec;
    if (fs::exists(generatedSrc, ec)) {
        fs::copy(generatedSrc, generatedDst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        if (ec) {
            setError("Failed to copy generated directory: " + ec.message());
            return false;
        }
    }

    // Also copy scene_scripts.cpp
    fs::path sceneScriptsSrc = project->getProjectInternalPath() / "scene_scripts.cpp";
    if (fs::exists(sceneScriptsSrc, ec)) {
        fs::copy_file(sceneScriptsSrc, config.targetDir / "generated" / "scene_scripts.cpp",
                      fs::copy_options::overwrite_existing, ec);
    }

    return true;
}

bool Editor::Exporter::copyAssets() {
    setProgress("Copying assets...", 0.3f);

    fs::path assetsSrc = config.assetsDir;
    fs::path assetsDst = config.targetDir / "assets";

    if (assetsSrc.empty()) {
        assetsSrc = project->getProjectPath();
    }

    std::error_code ec;
    if (!fs::exists(assetsSrc, ec)) {
        setError("Assets directory does not exist: " + assetsSrc.string());
        return false;
    }

    fs::create_directories(assetsDst, ec);

    // Collect C++ script paths to exclude from asset copy
    std::set<fs::path> scriptPaths;
    for (const auto& sceneProject : project->getScenes()) {
        for (const auto& script : sceneProject.cppScripts) {
            if (!script.path.empty()) {
                fs::path p = script.path;
                if (p.is_relative()) p = project->getProjectPath() / p;
                scriptPaths.insert(fs::weakly_canonical(p, ec));
            }
            if (!script.headerPath.empty()) {
                fs::path p = script.headerPath;
                if (p.is_relative()) p = project->getProjectPath() / p;
                scriptPaths.insert(fs::weakly_canonical(p, ec));
            }
        }
    }

    // Copy assets, excluding .supernova directory and C++ scripts
    for (auto& entry : fs::recursive_directory_iterator(assetsSrc, fs::directory_options::skip_permission_denied, ec)) {
        fs::path relativePath = fs::relative(entry.path(), assetsSrc, ec);

        // Skip .supernova internal directory and hidden dirs
        std::string firstComponent = relativePath.begin()->string();
        if (firstComponent == ".supernova" || firstComponent == ".vscode" || firstComponent == ".git") {
            continue;
        }
        // Skip CMakeLists.txt at root
        if (relativePath == "CMakeLists.txt") {
            continue;
        }
        // Skip C++ script files (already handled by copyCppScripts)
        if (entry.is_regular_file() && scriptPaths.count(fs::weakly_canonical(entry.path(), ec))) {
            continue;
        }

        fs::path destPath = assetsDst / relativePath;
        if (entry.is_directory()) {
            fs::create_directories(destPath, ec);
        } else if (entry.is_regular_file()) {
            fs::create_directories(destPath.parent_path(), ec);
            fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing, ec);
        }
    }

    return true;
}

bool Editor::Exporter::copyCppScripts() {
    setProgress("Copying C++ scripts...", 0.4f);

    fs::path scriptsDst = config.targetDir / "scripts";

    std::error_code ec;
    std::set<std::string> copiedPaths;

    for (const auto& sceneProject : project->getScenes()) {
        for (const auto& script : sceneProject.cppScripts) {
            // Copy source file
            if (!script.path.empty()) {
                fs::path srcPath = script.path;
                if (srcPath.is_relative()) {
                    srcPath = project->getProjectPath() / srcPath;
                }
                std::string pathKey = srcPath.string();
                if (copiedPaths.count(pathKey)) continue;
                copiedPaths.insert(pathKey);

                if (fs::exists(srcPath, ec)) {
                    fs::path relPath = fs::relative(srcPath, project->getProjectPath(), ec);
                    fs::path dstPath = scriptsDst / relPath;
                    fs::create_directories(dstPath.parent_path(), ec);
                    fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing, ec);
                }
            }

            // Copy header file
            if (!script.headerPath.empty()) {
                fs::path hdrPath = script.headerPath;
                if (hdrPath.is_relative()) {
                    hdrPath = project->getProjectPath() / hdrPath;
                }
                std::string pathKey = hdrPath.string();
                if (copiedPaths.count(pathKey)) continue;
                copiedPaths.insert(pathKey);

                if (fs::exists(hdrPath, ec)) {
                    fs::path relPath = fs::relative(hdrPath, project->getProjectPath(), ec);
                    fs::path dstPath = scriptsDst / relPath;
                    fs::create_directories(dstPath.parent_path(), ec);
                    fs::copy_file(hdrPath, dstPath, fs::copy_options::overwrite_existing, ec);
                }
            }
        }
    }

    return true;
}

bool Editor::Exporter::copyEngine() {
    setProgress("Copying engine...", 0.5f);

#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path engineSrc = fs::path(exePath).parent_path() / "engine";
#else
    fs::path engineSrc = fs::canonical("/proc/self/exe").parent_path() / "engine";
#endif
    fs::path engineDst = config.targetDir / "engine";

    std::error_code ec;
    if (fs::exists(engineSrc, ec)) {
        fs::copy(engineSrc, engineDst, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        if (ec) {
            setError("Failed to copy engine directory: " + ec.message());
            return false;
        }
    } else {
        setError("Engine directory not found at: " + engineSrc.string());
        return false;
    }

    return true;
}

bool Editor::Exporter::buildAndSaveShaders() {
    setProgress("Building shaders...", 0.6f);

    fs::path shadersDst = config.targetDir / "assets" / "shaders";

    std::error_code ec;
    fs::create_directories(shadersDst, ec);
    if (ec) {
        setError("Failed to create shaders directory: " + ec.message());
        return false;
    }

    struct ShaderFormat {
        supershader::lang_type_t lang;
        int version;
        bool es;
        supershader::platform_t platform;
        std::string suffix;
    };

    std::vector<ShaderFormat> requiredFormats;
    // Collect formats based on selected platforms
    if (config.selectedPlatforms.count(Platform::Linux) || config.selectedPlatforms.count(Platform::Windows)) {
        requiredFormats.push_back({supershader::LANG_GLSL, 410, false, supershader::PLATFORM_DEFAULT, "glsl410"});
    }
    if (config.selectedPlatforms.count(Platform::Android) || config.selectedPlatforms.count(Platform::Web)) {
        requiredFormats.push_back({supershader::LANG_GLSL, 300, true, supershader::PLATFORM_DEFAULT, "glsl300es"});
    }
    if (config.selectedPlatforms.count(Platform::MacOS)) {
        requiredFormats.push_back({supershader::LANG_MSL, 21, false, supershader::PLATFORM_MACOS, "msl21macos"});
    }
    if (config.selectedPlatforms.count(Platform::iOS)) {
        requiredFormats.push_back({supershader::LANG_MSL, 21, false, supershader::PLATFORM_IOS, "msl21ios"});
    }

    // Default to glsl410 if no platforms require anything
    if (requiredFormats.empty()) {
        requiredFormats.push_back({supershader::LANG_GLSL, 410, false, supershader::PLATFORM_DEFAULT, "glsl410"});
    }

    int total = (int)config.selectedShaderKeys.size() * requiredFormats.size();
    int current = 0;

    for (const ShaderKey& shaderKey : config.selectedShaderKeys) {
        ShaderType type = ShaderPool::getShaderTypeFromKey(shaderKey);
        uint32_t props = ShaderPool::getPropertiesFromKey(shaderKey);
        std::string shaderStr = ShaderPool::getShaderStr(type, props);

        for (const auto& fmt : requiredFormats) {
            float shaderProgress = 0.6f + (0.3f * (float)current / (float)std::max(total, 1));
            std::string fmtStr = fmt.suffix;
            setProgress("Building shader: " + shaderStr + " (" + fmtStr + ")", shaderProgress);

            try {
                // Synchronous build without cache
                ShaderData resultData = shaderBuilder.buildShaderForExport(shaderKey, fmt.lang, fmt.version, fmt.es, fmt.platform);

                std::string filename = shaderStr + "_" + fmtStr + ".sdat";
                fs::path outputPath = shadersDst / filename;

                std::string err;
                if (!ShaderDataSerializer::writeToFile(outputPath.string(), shaderKey, resultData, &err)) {
                    Out::warning("Failed to save shader %s: %s", filename.c_str(), err.c_str());
                }
            } catch (const std::exception& e) {
                Out::warning("Failed to build shader %s (%s): %s", shaderStr.c_str(), fmtStr.c_str(), e.what());
            }

            current++;
        }
    }

    return true;
}

bool Editor::Exporter::generateCMakeLists() {
    setProgress("Generating CMakeLists.txt...", 0.9f);

    std::string libName = "supernovaproject";

    // Collect script sources
    std::string scriptSources = "set(SCRIPT_SOURCES\n";
    std::set<std::string> scriptIncludeDirs;

    for (const auto& sceneProject : project->getScenes()) {
        for (const auto& script : sceneProject.cppScripts) {
            if (!script.path.empty()) {
                fs::path relPath = script.path;
                scriptSources += "    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/" + relPath.generic_string() + "\n";
            }
            if (!script.headerPath.empty()) {
                scriptIncludeDirs.insert("    ${CMAKE_CURRENT_SOURCE_DIR}/scripts\n");
            }
        }
    }
    scriptSources += ")\n";

    // Collect factory sources from generated directory
    std::string factorySources = "set(FACTORY_SOURCES\n";
    std::error_code ec;
    fs::path generatedDst = config.targetDir / "generated";
    if (fs::exists(generatedDst, ec)) {
        for (auto& entry : fs::directory_iterator(generatedDst, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                std::string filename = entry.path().filename().string();
                if (filename != "scene_scripts.cpp" && filename != "PlatformEditor.cpp" && filename != "main.cpp") {
                    factorySources += "    ${CMAKE_CURRENT_SOURCE_DIR}/generated/" + filename + "\n";
                }
            }
        }
    }
    factorySources += ")\n";

    std::string scriptDirs;
    for (const auto& dir : scriptIncludeDirs) {
        scriptDirs += dir;
    }

    std::string cmakeContent;
    cmakeContent += "# This file is auto-generated by Supernova Editor Export. Do not edit manually.\n\n";
    cmakeContent += "cmake_minimum_required(VERSION 3.15)\n";
    cmakeContent += "project(" + libName + ")\n\n";
    cmakeContent += "# Specify C++ standard\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD 17)\n";
    cmakeContent += "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    // Platform configuration
    cmakeContent += "add_definitions(\"-DDEFAULT_WINDOW_WIDTH=960\")\n";
    cmakeContent += "add_definitions(\"-DDEFAULT_WINDOW_HEIGHT=540\")\n\n";
    cmakeContent += "set(COMPILE_ZLIB OFF)\n";
    cmakeContent += "set(IS_ARM OFF)\n\n";
    cmakeContent += "add_definitions(\"-DSOKOL_GLCORE\")\n";
    cmakeContent += "add_definitions(\"-DWITH_MINIAUDIO\")\n\n";

    cmakeContent += "list(APPEND PLATFORM_SOURCE\n";
    cmakeContent += "    ${CMAKE_CURRENT_SOURCE_DIR}/generated/PlatformEditor.cpp\n";
    cmakeContent += "    ${CMAKE_CURRENT_SOURCE_DIR}/generated/main.cpp\n";
    cmakeContent += ")\n\n";

    cmakeContent += "list(APPEND PLATFORM_LIBS\n";
    cmakeContent += "    GL dl m glfw\n";
    cmakeContent += ")\n\n";

    cmakeContent += scriptSources + "\n";
    cmakeContent += factorySources + "\n";
    cmakeContent += "set(PROJECT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/generated/scene_scripts.cpp)\n\n";

    // Executable target
    cmakeContent += "if(NOT CMAKE_SYSTEM_NAME STREQUAL \"Android\")\n";
    cmakeContent += "    add_executable(" + libName + "\n";
    cmakeContent += "        ${PROJECT_SOURCE}\n";
    cmakeContent += "        ${SCRIPT_SOURCES}\n";
    cmakeContent += "        ${FACTORY_SOURCES}\n";
    cmakeContent += "        ${PLATFORM_SOURCE}\n";
    cmakeContent += "    )\n";
    cmakeContent += "else()\n";
    cmakeContent += "    add_library(" + libName + " SHARED\n";
    cmakeContent += "        ${PROJECT_SOURCE}\n";
    cmakeContent += "        ${SCRIPT_SOURCES}\n";
    cmakeContent += "        ${FACTORY_SOURCES}\n";
    cmakeContent += "        ${PLATFORM_SOURCE}\n";
    cmakeContent += "    )\n";
    cmakeContent += "endif()\n\n";

    // Suppress warnings in release
    cmakeContent += "if(NOT CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n";
    cmakeContent += "    set(SUPERNOVA_LIB_SYSTEM SYSTEM)\n";
    cmakeContent += "endif()\n\n";

    // Include directories
    std::string engineApiPath = "${CMAKE_CURRENT_SOURCE_DIR}/engine";
    cmakeContent += "target_include_directories(" + libName + " ${SUPERNOVA_LIB_SYSTEM} PRIVATE\n";
    cmakeContent += scriptDirs;
    cmakeContent += "    " + engineApiPath + "\n";
    cmakeContent += "    " + engineApiPath + "/libs/sokol\n";
    cmakeContent += "    " + engineApiPath + "/libs/box2d/include\n";
    cmakeContent += "    " + engineApiPath + "/libs/joltphysics\n";
    cmakeContent += "    " + engineApiPath + "/renders\n";
    cmakeContent += "    " + engineApiPath + "/core\n";
    cmakeContent += "    " + engineApiPath + "/core/action\n";
    cmakeContent += "    " + engineApiPath + "/core/buffer\n";
    cmakeContent += "    " + engineApiPath + "/core/component\n";
    cmakeContent += "    " + engineApiPath + "/core/ecs\n";
    cmakeContent += "    " + engineApiPath + "/core/io\n";
    cmakeContent += "    " + engineApiPath + "/core/manager\n";
    cmakeContent += "    " + engineApiPath + "/core/math\n";
    cmakeContent += "    " + engineApiPath + "/core/object\n";
    cmakeContent += "    " + engineApiPath + "/core/object/audio\n";
    cmakeContent += "    " + engineApiPath + "/core/object/ui\n";
    cmakeContent += "    " + engineApiPath + "/core/object/environment\n";
    cmakeContent += "    " + engineApiPath + "/core/object/physics\n";
    cmakeContent += "    " + engineApiPath + "/core/pool\n";
    cmakeContent += "    " + engineApiPath + "/core/registry\n";
    cmakeContent += "    " + engineApiPath + "/core/render\n";
    cmakeContent += "    " + engineApiPath + "/core/script\n";
    cmakeContent += "    " + engineApiPath + "/core/shader\n";
    cmakeContent += "    " + engineApiPath + "/core/subsystem\n";
    cmakeContent += "    " + engineApiPath + "/core/texture\n";
    cmakeContent += "    " + engineApiPath + "/core/util\n";
    cmakeContent += ")\n\n";

    // Supernova library
    cmakeContent += "# Set SUPERNOVA_LIB_DIR to the directory containing the Supernova library\n";
    cmakeContent += "if(NOT DEFINED SUPERNOVA_LIB_DIR OR SUPERNOVA_LIB_DIR STREQUAL \"\")\n";
    cmakeContent += "    message(FATAL_ERROR \"SUPERNOVA_LIB_DIR must be set to the directory containing the Supernova library\")\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "find_library(SUPERNOVA_LIB supernova PATHS ${SUPERNOVA_LIB_DIR} NO_DEFAULT_PATH)\n";
    cmakeContent += "if(NOT SUPERNOVA_LIB)\n";
    cmakeContent += "    message(FATAL_ERROR \"Supernova library not found in ${SUPERNOVA_LIB_DIR}\")\n";
    cmakeContent += "endif()\n\n";
    cmakeContent += "target_link_libraries(" + libName + " PRIVATE ${SUPERNOVA_LIB} ${PLATFORM_LIBS})\n\n";

    // Compile options
    cmakeContent += "if(MSVC)\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE /W4 /EHsc)\n";
    cmakeContent += "else()\n";
    cmakeContent += "    target_compile_options(" + libName + " PRIVATE -Wall -Wextra -fPIC)\n";
    cmakeContent += "endif()\n\n";

    // Output properties
    cmakeContent += "set_target_properties(" + libName + " PROPERTIES\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}\n";
    cmakeContent += "    OUTPUT_NAME \"" + libName + "\"\n";
    cmakeContent += ")\n";

    fs::path cmakeFile = config.targetDir / "CMakeLists.txt";
    FileUtils::writeIfChanged(cmakeFile, cmakeContent);

    return true;
}

// Static helpers for UI display

std::string Editor::Exporter::getShaderDisplayName(ShaderType type, uint32_t properties) {
    std::string name = ShaderPool::getShaderTypeName(type);
    int propCount = ShaderPool::getShaderPropertyCount(type);

    std::string props;
    for (int i = 0; i < propCount; i++) {
        if (properties & (1 << i)) {
            if (!props.empty()) props += ", ";
            props += ShaderPool::getShaderPropertyName(type, i, true);
        }
    }

    if (!props.empty()) {
        name += " (" + props + ")";
    }

    return name;
}

std::string Editor::Exporter::getPlatformName(Platform platform) {
    switch (platform) {
        case Platform::MacOS:   return "macOS";
        case Platform::iOS:     return "iOS";
        case Platform::Web:     return "Web";
        case Platform::Android: return "Android";
        case Platform::Linux:   return "Linux";
        case Platform::Windows: return "Windows";
        default:                return "Unknown";
    }
}
