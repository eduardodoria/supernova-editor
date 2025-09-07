#pragma once

#include "command/Command.h"
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class RenameFileCmd: public Command{

    private:

        fs::path oldFilename;
        fs::path newFilename;
        fs::path directory;

    public:
        RenameFileCmd(std::string oldName, std::string newName, std::string directory);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}