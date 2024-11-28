#ifndef CHANGEENTITYNAMECMD_H
#define CHANGEENTITYNAMECMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class ChangeEntityNameCmd: public Command{

    private:
        std::string oldName;
        std::string newName;

        Scene* scene;
        Entity entity;

    public:
        ChangeEntityNameCmd(Scene* scene, Entity entity, std::string name);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CHANGEENTITYNAMECMD_H */