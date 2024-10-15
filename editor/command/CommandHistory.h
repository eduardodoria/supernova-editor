#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include "Command.h"
#include <vector>

namespace Supernova::Editor{

    class CommandHistory{

    private:
        std::vector<Command*> list;
        size_t index;

    public:
        CommandHistory();

        void addCommand(Command* cmd);

        void undo();
        void redo();
    };

}

#endif /* COMMANDHISTORY_H */