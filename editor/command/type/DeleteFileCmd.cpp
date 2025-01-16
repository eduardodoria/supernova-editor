#include "DeleteFileCmd.h"
#include <filesystem>
#include <string>

using namespace Supernova;

Editor::DeleteFileCmd::DeleteFileCmd(std::vector<fs::path> filePaths, fs::path rootPath){
    for (fs::path& file : filePaths){
        this->files.push_back({file, fs::path()});
    }
    this->trash = rootPath / ".trash";
}

fs::path Editor::DeleteFileCmd::generateUniqueTrashPath(const fs::path& trashDir, const fs::path& originalFile) {
    fs::path basePath = trashDir / originalFile.filename();
    fs::path extension = originalFile.extension();
    fs::path stemPath = basePath.stem();
    
    if (!fs::exists(basePath)) {
        return basePath;
    }

    int counter = 1;
    fs::path newPath;
    do {
        newPath = trashDir / (stemPath.string() + " (" + std::to_string(counter) + ")" + extension.string());
        counter++;
    } while (fs::exists(newPath));

    return newPath;
}

void Editor::DeleteFileCmd::execute(){
    if (!fs::exists(trash)) {
        fs::create_directory(trash);
    }

    for (DeleteFilesData& file : files){
        try {
            if (fs::exists(file.originalFile)) {
                // Check if file is inside .trash directory or is .trash directory
                if (file.originalFile == trash || 
                    fs::relative(file.originalFile, trash).native().find("..") == std::string::npos) {
                    if (fs::is_directory(file.originalFile)) {
                        fs::remove_all(file.originalFile);
                    } else {
                        fs::remove(file.originalFile);
                    }
                } else {
                    // Normal deletion - move to trash with unique name
                    if (!fs::exists(trash)) {
                        fs::create_directory(trash);
                    }
                    file.trashFile = generateUniqueTrashPath(trash, file.originalFile);
                    fs::rename(file.originalFile, file.trashFile);
                }
            }
        } catch (const fs::filesystem_error& e) {
            printf("Error: Deleting %s: %s\n", file.originalFile.c_str(), e.what());
        }
    }
}

void Editor::DeleteFileCmd::undo(){
    for (DeleteFilesData& file : files){
        try {
            // Only attempt to restore if the file was moved to trash (not permanently deleted)
            if (!file.trashFile.empty() && fs::exists(file.trashFile)) {
                fs::rename(file.trashFile, file.originalFile);
            }
        } catch (const fs::filesystem_error& e) {
            printf("Error: Restoring %s: %s\n", file.originalFile.c_str(), e.what());
        }
    }
}

bool Editor::DeleteFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}