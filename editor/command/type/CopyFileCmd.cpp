#include "CopyFileCmd.h"

using namespace Supernova;

Editor::CopyFileCmd::CopyFileCmd(std::vector<std::string> sourceFiles, std::string currentDirectory, std::string targetDirectory, bool remove){
    for (const auto& sourceFile : sourceFiles) {
        FileCopyData fdata;
        fdata.filename = fs::path(sourceFile).filename();
        fdata.sourceDirectory = fs::path(currentDirectory);
        fdata.targetDirectory = fs::path(targetDirectory);

        this->files.push_back(fdata);
    }
    this->remove = remove;
}

Editor::CopyFileCmd::CopyFileCmd(std::vector<std::string> sourcePaths, std::string targetDirectory, bool remove){
    for (const auto& sourcePath : sourcePaths) {
        FileCopyData fdata;
        fdata.filename = fs::path(sourcePath).filename();
        fdata.sourceDirectory = fs::path(sourcePath).remove_filename();
        fdata.targetDirectory = fs::path(targetDirectory);

        this->files.push_back(fdata);
    }
    this->remove = remove;
}

void Editor::CopyFileCmd::execute(){
    for (const auto& fdata : files) {
        fs::path sourceFs = fdata.sourceDirectory / fdata.filename;
        fs::path destFs = fdata.targetDirectory / fdata.filename;
        try {
            if (fs::exists(sourceFs)) {
                if (fs::is_directory(sourceFs)) {
                    fs::copy(sourceFs, destFs, fs::copy_options::recursive);
                    if (remove) {
                        fs::remove_all(sourceFs);
                    }
                } else {
                    fs::copy(sourceFs, destFs, fs::copy_options::overwrite_existing);
                    if (remove) {
                        fs::remove(sourceFs);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            // Handle error if needed
        }
    }
}

void Editor::CopyFileCmd::undo(){
    for (const auto& fdata : files) {
        fs::path sourceFs = fdata.targetDirectory / fdata.filename;
        fs::path destFs = fdata.sourceDirectory / fdata.filename;
        try {
            if (fs::exists(sourceFs)) {
                if (fs::is_directory(sourceFs)) {
                    if (remove) {
                        fs::copy(sourceFs, destFs, fs::copy_options::recursive);
                    }
                    fs::remove_all(sourceFs);
                } else {
                    if (remove) {
                        fs::copy(sourceFs, destFs, fs::copy_options::overwrite_existing);
                    }
                    fs::remove(sourceFs);
                }
            }
        } catch (const fs::filesystem_error& e) {
            // Handle error if needed
        }
    }
}

bool Editor::CopyFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}