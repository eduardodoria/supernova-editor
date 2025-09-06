#pragma once

#include "command/Command.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class CreateDirCmd: public Command{

    private:

        fs::path directory;

    public:
        CreateDirCmd(std::string dirName, std::string dirPath);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}