#ifndef CREATEENTITYCOMMAND_H
#define CREATEENTITYCOMMAND_H

#include "Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class CreateEntityCommand: public Command{

    public:
        CreateEntityCommand(Project* project, uint32_t sceneId, std::string entityName);

        virtual void execute();
        virtual void undo();
    };

}

#endif /* CREATEENTITYCOMMAND_H */