#include "RenameFileCmd.h"

using namespace Supernova;

Editor::RenameFileCmd::RenameFileCmd(std::string oldName, std::string newName, std::string directory){
    this->oldFilename = fs::path(oldName).filename();
    this->newFilename = fs::path(newName).filename();
    this->directory = fs::path(directory);
}

void Editor::RenameFileCmd::execute(){
    fs::path sourceFs = directory / oldFilename;
    fs::path destFs = directory / newFilename;
    try {
        if (fs::exists(sourceFs)) {
            fs::rename(sourceFs, destFs);
        }
    } catch (const fs::filesystem_error& e) {
        printf("Error: Renaming %s: %s\n", sourceFs.c_str(), e.what());
    }
}

void Editor::RenameFileCmd::undo(){
    fs::path sourceFs = directory / newFilename;
    fs::path destFs = directory / oldFilename;
    try {
        if (fs::exists(sourceFs)) {
            fs::rename(sourceFs, destFs);
        }
    } catch (const fs::filesystem_error& e) {
        printf("Error: Undo renaming %s: %s\n", sourceFs.c_str(), e.what());
    }
}

bool Editor::RenameFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}