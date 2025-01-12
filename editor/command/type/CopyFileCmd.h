#ifndef COPYFILECMD_H
#define COPYFILECMD_H

#include "command/Command.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    struct FileCopyData{
        fs::path filename;
        fs::path sourceDirectory;
        fs::path targetDirectory;
    };

    class CopyFileCmd: public Command{

    private:

        std::vector<FileCopyData> files;
        bool remove;

    public:
        CopyFileCmd(std::vector<std::string> sourceFiles, std::string currentDirectory, std::string targetDirectory, bool remove);
        CopyFileCmd(std::vector<std::string> sourcePaths, std::string targetDirectory, bool remove);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* COPYFILECMD_H */