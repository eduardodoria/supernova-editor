#ifndef DELETEFILECMD_H
#define DELETEFILECMD_H

#include "command/Command.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class DeleteFileCmd: public Command{

    private:

        std::vector<fs::path> files;

        std::string executeCommand(const char* cmd);
        bool isCommandAvailable(const std::string& command);
        fs::path getTrashPath();
        bool moveToTrash(const fs::path& path);
        bool restoreFromTrash(const fs::path& fileName);

    public:
        DeleteFileCmd(std::vector<fs::path> files);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* DELETEFILECMD_H */