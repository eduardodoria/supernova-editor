#include "CopyFileCmd.h"

using namespace Supernova;

Editor::CopyFileCmd::CopyFileCmd(std::vector<std::string> sourceFiles, std::string currentDirectory, std::string targetDirectory, bool copy){
    for (const auto& sourceFile : sourceFiles) {
        FileCopyData fdata;
        fdata.filename = fs::path(sourceFile).filename();
        fdata.sourceDirectory = fs::path(currentDirectory);
        fdata.targetDirectory = fs::path(targetDirectory);

        this->files.push_back(fdata);
    }
    this->copy = copy;
}

Editor::CopyFileCmd::CopyFileCmd(std::vector<std::string> sourcePaths, std::string targetDirectory, bool copy){
    for (const auto& sourcePath : sourcePaths) {
        FileCopyData fdata;
        fdata.filename = fs::path(sourcePath).filename();
        fdata.sourceDirectory = fs::path(sourcePath).remove_filename();
        fdata.targetDirectory = fs::path(targetDirectory);

        this->files.push_back(fdata);
    }
    this->copy = copy;
}

void Editor::CopyFileCmd::execute(){
    for (const auto& fdata : files) {
        fs::path sourceFs = fdata.sourceDirectory / fdata.filename;
        fs::path destFs = fdata.targetDirectory / fdata.filename;
        try {
            if (fs::exists(sourceFs)) {
                if (fs::is_directory(sourceFs)) {
                    if (copy){
                        fs::copy(sourceFs, destFs, fs::copy_options::recursive);
                    }else{
                        fs::rename(sourceFs, destFs);
                    }
                } else {
                    if (copy){
                        fs::copy(sourceFs, destFs, fs::copy_options::overwrite_existing);
                    }else{
                        fs::rename(sourceFs, destFs);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            printf("Error: Moving/Copying %s: %s\n", sourceFs.string().c_str(), e.what());
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
                    if (copy) {
                        fs::remove_all(sourceFs);
                    }else{
                        fs::rename(sourceFs, destFs);
                    }
                } else {
                    if (copy) {
                        fs::remove(sourceFs);
                    }else{
                        fs::rename(sourceFs, destFs);
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            printf("Error: Undo moving/Copying %s: %s\n", sourceFs.string().c_str(), e.what());
        }
    }
}

bool Editor::CopyFileCmd::mergeWith(Editor::Command* otherCommand){
    return false;
}