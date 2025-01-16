#ifndef DELETEFILECMD_H
#define DELETEFILECMD_H

#include "command/Command.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class DeleteFileCmd: public Command{

        struct DeleteFilesData{
            fs::path originalFile;
            fs::path trashFile;
        };

    private:

        std::vector<DeleteFilesData> files;
        fs::path trash;

        fs::path generateUniqueTrashPath(const fs::path& trashDir, const fs::path& originalFile);

    public:
        DeleteFileCmd(std::vector<fs::path> filePaths, fs::path rootPath);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* DELETEFILECMD_H */