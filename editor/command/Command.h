#ifndef COMMAND_H
#define COMMAND_H

namespace Supernova::Editor{

    class Command{
    protected:
        bool mergeable = true;

    public:
        virtual ~Command() {}

        virtual void execute() = 0;
        virtual void undo() = 0;

        virtual bool mergeWith(Command* olderCommand) = 0;

        void setNoMerge() { mergeable = false; }
        bool canMerge() const { return mergeable; }
    };

}

#endif /* COMMAND_H */