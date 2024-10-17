#ifndef CREATEENTITYCMD_H
#define CREATEENTITYCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class CreateEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        std::string entityName;

        Entity entity;

    public:
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CREATEENTITYCMD_H */