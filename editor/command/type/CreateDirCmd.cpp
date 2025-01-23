#include "CreateDirCmd.h"

using namespace Supernova;

Editor::CreateDirCmd::CreateDirCmd(std::string dirName, std::string dirPath){
    this->directory = fs::path(dirPath) / fs::path(dirName);
}

void Editor::CreateDirCmd::execute(){
    try {
        fs::create_directory(directory);
    } catch (const fs::filesystem_error& e) {
        printf("Error: Creating directory %s: %s\n", directory.string().c_str(), e.what());
    }
}

void Editor::CreateDirCmd::undo(){
    try {
        fs::remove_all(directory);
    } catch (const fs::filesystem_error& e) {
        printf("Error: Undo creating directory %s: %s\n", directory.string().c_str(), e.what());
    }
}

bool Editor::CreateDirCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}