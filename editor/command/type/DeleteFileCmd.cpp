#include "DeleteFileCmd.h"

#include <filesystem>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <array>
#include <fstream>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
#endif

using namespace Supernova;

Editor::DeleteFileCmd::DeleteFileCmd(std::vector<fs::path> files){
    this->files = files;
}

void Editor::DeleteFileCmd::execute(){
    for (fs::path& file : files){
        moveToTrash(file);
    }
}

void Editor::DeleteFileCmd::undo(){
    for (fs::path& file : files){
        restoreFromTrash(file);
    }
}

bool Editor::DeleteFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}


std::string Editor::DeleteFileCmd::executeCommand(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe;

    #ifdef _WIN32
        pipe = _popen(cmd, "r");
    #else
        pipe = popen(cmd, "r");
    #endif
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif

    return result;
}

bool Editor::DeleteFileCmd::isCommandAvailable(const std::string& command) {
    #ifdef _WIN32
        return true;
    #else
        std::string cmd = "which " + command + " > /dev/null 2>&1";
        return std::system(cmd.c_str()) == 0;
    #endif
}

fs::path Editor::DeleteFileCmd::getTrashPath() {
    try {
        #ifdef _WIN32
            // Windows: Get Recycle Bin path
            PWSTR path = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RecycleBinFolder, 0, nullptr, &path))) {
                fs::path result(path);
                CoTaskMemFree(path);
                return result;
            }
            throw std::runtime_error("Failed to get Recycle Bin path");

        #elif defined(__APPLE__)
            // macOS: Try multiple methods to get trash path
            const char* homeDir = std::getenv("HOME");
            if (!homeDir) {
                homeDir = getpwuid(getuid())->pw_dir;
            }
            
            if (!homeDir) {
                throw std::runtime_error("Unable to determine home directory");
            }

            fs::path trashPath = fs::path(homeDir) / ".Trash";
            
            // Check if .Trash exists and is accessible
            if (!fs::exists(trashPath)) {
                // Try to create .Trash directory
                std::error_code ec;
                fs::create_directories(trashPath, ec);
                if (ec) {
                    // Fall back to volume-specific trash
                    fs::path volumeTrash = "/.Trashes";
                    if (fs::exists(volumeTrash) && fs::is_directory(volumeTrash)) {
                        return volumeTrash;
                    }
                    throw std::runtime_error("Unable to access or create Trash directory");
                }
            }
            return trashPath;

        #else
            // Linux/Unix systems: Follow FreeDesktop.org trash specification
            const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
            fs::path trashPath;

            if (xdgDataHome && *xdgDataHome) {
                trashPath = fs::path(xdgDataHome) / "Trash";
            } else {
                const char* homeDir = std::getenv("HOME");
                if (!homeDir) {
                    homeDir = getpwuid(getuid())->pw_dir;
                }
                
                if (!homeDir) {
                    throw std::runtime_error("Unable to determine home directory");
                }

                trashPath = fs::path(homeDir) / ".local/share/Trash";
            }

            // Check for alternative trash locations
            if (!fs::exists(trashPath)) {
                // Try standard KDE location
                const char* homeDir = std::getenv("HOME");
                if (homeDir) {
                    fs::path kdePath = fs::path(homeDir) / ".kde/share/apps/ktrash";
                    if (fs::exists(kdePath)) {
                        return kdePath;
                    }
                }

                // Try to create the default trash directory
                std::error_code ec;
                fs::create_directories(trashPath, ec);
                if (ec) {
                    // Check if we're on a different partition
                    fs::path topLevelTrash = fs::path("/") / ".Trash";
                    if (fs::exists(topLevelTrash) && fs::is_directory(topLevelTrash)) {
                        // Use UID-specific trash for security
                        uid_t uid = getuid();
                        fs::path uidTrash = topLevelTrash / std::to_string(uid);
                        return uidTrash;
                    }
                    
                    throw std::runtime_error("Unable to access or create Trash directory");
                }
            }

            // Verify trash directory structure
            fs::path filesDir = trashPath / "files";
            fs::path infoDir = trashPath / "info";
            
            std::error_code ec;
            fs::create_directories(filesDir, ec);
            fs::create_directories(infoDir, ec);
            
            // Check permissions
            if (!fs::exists(filesDir) || !fs::exists(infoDir)) {
                throw std::runtime_error("Unable to create required trash subdirectories");
            }

            // Verify write permissions
            auto testFile = filesDir / ".test";
            {
                std::ofstream test(testFile);
                if (!test.is_open()) {
                    throw std::runtime_error("No write permission in trash directory");
                }
            }
            fs::remove(testFile, ec);

            return trashPath;
        #endif

    } catch (const std::exception& e) {
        std::cerr << "Error in getTrashPath: " << e.what() << std::endl;
        return fs::path();
    }
}

bool Editor::DeleteFileCmd::moveToTrash(const fs::path& path) {
    try {
        if (!fs::exists(path)) {
            std::cerr << "File does not exist: " << path << std::endl;
            return false;
        }

        if (fs::is_symlink(path)) {
            std::cerr << "Warning: Moving symbolic link to trash" << std::endl;
        }

        const fs::path absolutePath = fs::canonical(path);  // Resolves symlinks and ".." components
        bool success = false;

        #ifdef _WIN32
                std::error_code ec;
                if (fs::file_size(absolutePath, ec) > (1ULL << 40)) { // 1 TB
                    std::cerr << "Warning: Moving very large file to trash" << std::endl;
                }

                WCHAR from[MAX_PATH];
                if (absolutePath.wstring().length() >= MAX_PATH) {
                    std::cerr << "Path too long for Windows API" << std::endl;
                    return false;
                }
                
                wcscpy_s(from, MAX_PATH, absolutePath.wstring().c_str());
                from[absolutePath.wstring().length() + 1] = 0;  // Double null termination required
                
                SHFILEOPSTRUCTW fileOp = {0};
                fileOp.wFunc = FO_DELETE;
                fileOp.pFrom = from;
                fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
                
                int result = SHFileOperationW(&fileOp);
                if (result != 0) {
                    std::cerr << "Windows trash operation failed with code: " << result << std::endl;
                    return false;
                }
                success = true;

        #elif defined(__APPLE__)
                std::string escapedPath = absolutePath.string();
                // Escape special characters for AppleScript
                size_t pos = 0;
                while ((pos = escapedPath.find("\"", pos)) != std::string::npos) {
                    escapedPath.replace(pos, 1, "\\\"");
                    pos += 2;
                }

                std::string command = "osascript -e 'tell app \"Finder\" to delete POSIX file \"" 
                                + escapedPath + "\"'";
                success = (std::system(command.c_str()) == 0);
                if (!success) {
                    std::cerr << "Failed to move file to trash using Finder" << std::endl;
                }

        #else
                if (isCommandAvailable("gio")) {
                    std::string escapedPath = absolutePath.string();
                    // Escape special characters for shell
                    size_t pos = 0;
                    while ((pos = escapedPath.find("\"", pos)) != std::string::npos) {
                        escapedPath.replace(pos, 1, "\\\"");
                        pos += 2;
                    }

                    std::string command = "gio trash \"" + escapedPath + "\"";
                    success = (std::system(command.c_str()) == 0);
                    if (!success) {
                        std::cerr << "gio trash command failed" << std::endl;
                    }
                }
                else if (isCommandAvailable("gvfs-trash")) {
                    std::string escapedPath = absolutePath.string();
                    size_t pos = 0;
                    while ((pos = escapedPath.find("\"", pos)) != std::string::npos) {
                        escapedPath.replace(pos, 1, "\\\"");
                        pos += 2;
                    }

                    std::string command = "gvfs-trash \"" + escapedPath + "\"";
                    success = (std::system(command.c_str()) == 0);
                    if (!success) {
                        std::cerr << "gvfs-trash command failed" << std::endl;
                    }
                }
                else {
                    fs::path trashPath = getTrashPath();
                    if (trashPath.empty()) {
                        std::cerr << "Failed to determine trash path" << std::endl;
                        return false;
                    }

                    fs::path filesPath = trashPath / "files";
                    fs::path infoPath = trashPath / "info";

                    std::error_code ec;
                    fs::create_directories(filesPath, ec);
                    fs::create_directories(infoPath, ec);
                    if (ec) {
                        std::cerr << "Failed to create trash directories: " << ec.message() << std::endl;
                        return false;
                    }

                    fs::path fileName = path.filename();
                    fs::path trashedFilePath = filesPath / fileName;
                    int counter = 1;
                    
                    while (fs::exists(trashedFilePath)) {
                        std::string newName = fileName.stem().string() + "_" + 
                                            std::to_string(counter++) + 
                                            fileName.extension().string();
                        trashedFilePath = filesPath / newName;
                    }

                    try {
                        fs::rename(absolutePath, trashedFilePath);
                        fs::path trashInfoPath = infoPath / (trashedFilePath.filename().string() + ".trashinfo");
                        
                        auto now = std::chrono::system_clock::now();
                        auto now_time_t = std::chrono::system_clock::to_time_t(now);
                        char timeStr[100];
                        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", std::localtime(&now_time_t));

                        std::ofstream trashInfo(trashInfoPath);
                        if (!trashInfo.is_open()) {
                            std::cerr << "Failed to create trash info file" << std::endl;
                            // Try to undo the move
                            fs::rename(trashedFilePath, absolutePath);
                            return false;
                        }

                        trashInfo << "[Trash Info]\n"
                                << "Path=" << absolutePath.string() << "\n"
                                << "DeletionDate=" << timeStr << "\n";
                        success = true;
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        std::cerr << "Filesystem error: " << e.what() << std::endl;
                        return false;
                    }
                }
        #endif
        return success;

    } catch (const std::exception& e) {
        std::cerr << "Exception in moveToTrash: " << e.what() << std::endl;
        return false;
    }
}

bool Editor::DeleteFileCmd::restoreFromTrash(const fs::path& path) {
    if (path.empty()) {
        std::cerr << "Empty path provided for restoration" << std::endl;
        return false;
    }

    const fs::path fileName = path.filename();
    if (fileName.empty()) {
        std::cerr << "Invalid filename in path: " << path << std::endl;
        return false;
    }

    bool success = false;
    const fs::path absolutePath = fs::absolute(path);
    std::error_code ec;

    #ifdef _WIN32
        CoInitialize(nullptr);
        IFileOperation* fileOp = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileOp)))) {
            std::unique_ptr<IFileOperation, decltype(&IUnknown::Release)> fileOpPtr(fileOp, &IUnknown::Release);
            
            fileOpPtr->SetOperationFlags(FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT);
            
            IShellItem* recyclerItem = nullptr;
            if (SUCCEEDED(SHGetKnownFolderItem(FOLDERID_RecycleBinFolder, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&recyclerItem)))) {
                std::unique_ptr<IShellItem, decltype(&IUnknown::Release)> recyclerPtr(recyclerItem, &IUnknown::Release);
                
                IEnumShellItems* enumItems = nullptr;
                if (SUCCEEDED(recyclerPtr->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&enumItems)))) {
                    std::unique_ptr<IEnumShellItems, decltype(&IUnknown::Release)> enumPtr(enumItems, &IUnknown::Release);
                    
                    IShellItem* item;
                    while (enumPtr->Next(1, &item, nullptr) == S_OK && !success) {
                        std::unique_ptr<IShellItem, decltype(&IUnknown::Release)> itemPtr(item, &IUnknown::Release);
                        
                        PWSTR displayName = nullptr;
                        if (SUCCEEDED(itemPtr->GetDisplayName(SIGDN_NORMALDISPLAY, &displayName))) {
                            std::unique_ptr<WCHAR, decltype(&CoTaskMemFree)> displayNamePtr(displayName, &CoTaskMemFree);
                            
                            if (fileName == displayName) {
                                if (SUCCEEDED(fileOpPtr->CopyItem(itemPtr.get(), nullptr, nullptr, nullptr))) {
                                    success = SUCCEEDED(fileOpPtr->PerformOperations());
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        CoUninitialize();

    #elif defined(__APPLE__)
        // Create a unique temporary file to store the original path
        fs::path tempFile = fs::temp_directory_path() / ("temp_" + std::to_string(std::hash<std::string>{}(fileName.string())));
        
        std::string getPathCmd = "osascript -e 'tell app \"Finder\" to set origPath to (original item of (first item of (items of trash whose name is \"" 
                               + fileName.string() + "\"))) as text' > \"" + tempFile.string() + "\"";
        
        if (std::system(getPathCmd.c_str()) == 0) {
            std::ifstream tempFileStream(tempFile);
            std::string originalPath;
            if (std::getline(tempFileStream, originalPath)) {
                std::string restoreCmd = "osascript -e 'tell app \"Finder\" to restore (first item of (items of trash whose name is \"" 
                                      + fileName.string() + "\"))'";
                success = (std::system(restoreCmd.c_str()) == 0);
            }
            fs::remove(tempFile, ec);
        }

    #else
        if (isCommandAvailable("gio")) {
            // Get all items in trash with their URIs
            std::string cmd = "gio list -a standard::display-name trash:/// 2>/dev/null";
            std::string output = executeCommand(cmd.c_str());
            std::istringstream iss(output);
            std::string line;
            std::string trashUri;

            while (std::getline(iss, line)) {
                if (line.find(fileName.string()) != std::string::npos) {
                    size_t pos = line.find("trash:///");
                    if (pos != std::string::npos) {
                        trashUri = line.substr(pos);
                        // Remove any trailing whitespace
                        trashUri = trashUri.substr(0, trashUri.find_last_not_of(" \n\r\t") + 1);
                        break;
                    }
                }
            }

            if (!trashUri.empty()) {
                std::string restoreCmd = "gio trash --restore \"" + trashUri + "\" 2>/dev/null";
                success = (std::system(restoreCmd.c_str()) == 0);
            }
        }

        if (!success) {
            fs::path trashPath = getTrashPath();
            if (trashPath.empty()) {
                std::cerr << "Could not determine trash path" << std::endl;
                return false;
            }

            fs::path filesPath = trashPath / "files";
            fs::path infoPath = trashPath / "info";
            fs::path trashInfoPath = infoPath / (fileName.string() + ".trashinfo");

            if (!fs::exists(trashInfoPath)) {
                std::cerr << "Trash info file not found: " << trashInfoPath << std::endl;
                return false;
            }

            std::string originalPath;
            {
                std::ifstream trashInfo(trashInfoPath);
                std::string line;
                while (std::getline(trashInfo, line)) {
                    if (line.substr(0, 5) == "Path=") {
                        originalPath = line.substr(5);
                        break;
                    }
                }
            }

            if (!originalPath.empty()) {
                fs::path trashedFilePath = filesPath / fileName;
                if (fs::exists(trashedFilePath)) {
                    try {
                        fs::path destPath(originalPath);
                        fs::create_directories(destPath.parent_path(), ec);
                        
                        if (fs::exists(destPath)) {
                            // If file already exists, create a numbered backup
                            int counter = 1;
                            fs::path backupPath = destPath;
                            while (fs::exists(backupPath)) {
                                backupPath = destPath.parent_path() / (destPath.stem().string() + "_" + 
                                           std::to_string(counter++) + destPath.extension().string());
                            }
                            destPath = backupPath;
                        }

                        fs::rename(trashedFilePath, destPath, ec);
                        if (!ec) {
                            fs::remove(trashInfoPath, ec);
                            success = true;
                        } else {
                            std::cerr << "Error restoring file: " << ec.message() << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Exception during restore: " << e.what() << std::endl;
                    }
                }
            }
        }
    #endif

    return success;
}