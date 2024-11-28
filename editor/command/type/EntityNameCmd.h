#ifndef CHANGEENTITYNAMECMD_H
#define CHANGEENTITYNAMECMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class EntityNameCmd: public Command{

    private:
        std::string oldName;
        std::string newName;

        Scene* scene;
        Entity entity;

    public:
        EntityNameCmd(Scene* scene, Entity entity, std::string name);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CHANGEENTITYNAMECMD_H */