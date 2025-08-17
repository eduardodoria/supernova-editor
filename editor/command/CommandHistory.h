#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include "Command.h"
#include <vector>
#include <cstddef>

namespace Supernova::Editor{

    class CommandHistory{

    private:
        std::vector<Command*> list;
        size_t index; // real index is (index-1)

    public:
        virtual ~CommandHistory();

        void addCommand(Command* cmd);
        void addCommandNoMerge(Command* cmd);
        void addCommandCommit(Command* cmd);

        void undo();
        void redo();
    };

}

#endif /* COMMANDHISTORY_H */