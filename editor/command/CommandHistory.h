#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include "Command.h"
#include <vector>
#include <cstddef>

namespace Supernova::Editor{

    class CommandHistory{

    private:
        static std::vector<Command*> list;
        static size_t index; // real index is (index-1)

    public:
        virtual ~CommandHistory();

        static void addCommand(Command* cmd);

        static void undo();
        static void redo();
    };

}

#endif /* COMMANDHISTORY_H */