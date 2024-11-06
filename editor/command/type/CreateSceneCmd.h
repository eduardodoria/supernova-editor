#ifndef CREATESCENECMD_H
#define CREATESCENECMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class CreateSceneCmd: public Command{

    private:
        Project* project;
        std::string sceneName;

    public:
        CreateSceneCmd(Project* project, std::string sceneName);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CREATESCENECMD_H */