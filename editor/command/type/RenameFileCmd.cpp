#include "RenameFileCmd.h"

#include "util/Util.h"

using namespace Supernova;

Editor::RenameFileCmd::RenameFileCmd(Project* project, std::string oldName, std::string newName, std::string directory){
    this->project = project;
    this->oldFilename = fs::path(oldName).filename();
    this->newFilename = fs::path(newName).filename();
    this->directory = fs::path(directory);
}

bool Editor::RenameFileCmd::execute(){
    fs::path sourceFs = directory / oldFilename;
    fs::path destFs = directory / newFilename;
    try {
        if (fs::exists(sourceFs)) {
            bool isDir = fs::is_directory(sourceFs);
            fs::rename(sourceFs, destFs);
            if (project) {
                std::string extension = sourceFs.extension().string();
                if (isDir || Util::isMaterialFile(extension)) {
                    project->remapMaterialFilePath(sourceFs, destFs);
                }
                if (isDir || Util::isSceneFile(extension)) {
                    project->remapSceneFilePath(sourceFs, destFs);
                }
                if (isDir || Util::isEntityFile(extension)) {
                    project->remapSharedEntityFilePath(sourceFs, destFs);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        printf("Error: Renaming %s: %s\n", sourceFs.string().c_str(), e.what());
        return false;
    }

    return true;
}

void Editor::RenameFileCmd::undo(){
    fs::path sourceFs = directory / newFilename;
    fs::path destFs = directory / oldFilename;
    try {
        if (fs::exists(sourceFs)) {
            bool isDir = fs::is_directory(sourceFs);
            fs::rename(sourceFs, destFs);
            if (project) {
                std::string extension = sourceFs.extension().string();
                if (isDir || Util::isMaterialFile(extension)) {
                    project->remapMaterialFilePath(sourceFs, destFs);
                }
                if (isDir || Util::isSceneFile(extension)) {
                    project->remapSceneFilePath(sourceFs, destFs);
                }
                if (isDir || Util::isEntityFile(extension)) {
                    project->remapSharedEntityFilePath(sourceFs, destFs);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        printf("Error: Undo renaming %s: %s\n", sourceFs.string().c_str(), e.what());
    }
}

bool Editor::RenameFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}