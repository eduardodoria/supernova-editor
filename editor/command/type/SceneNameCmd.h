#ifndef SCENENAMECMD_H
#define SCENENAMECMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class SceneNameCmd: public Command{

    private:
        std::string oldName;
        std::string newName;

        Project* project;
        uint32_t sceneId;

    public:
        SceneNameCmd(Project* project, uint32_t sceneId, std::string name);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* SCENENAMECMD_H */