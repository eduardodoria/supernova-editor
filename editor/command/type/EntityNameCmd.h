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

        SceneProject* sceneProject;
        Entity entity;

        bool wasModified;

    public:
        EntityNameCmd(SceneProject* sceneProject, Entity entity, std::string name);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CHANGEENTITYNAMECMD_H */