#ifndef CREATEENTITYCMD_H
#define CREATEENTITYCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    enum class EntityCreationType{
        EMPTY,
        BOX
    };

    class CreateEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        std::string entityName;

        Entity entity;
        EntityCreationType type;
        Entity lastSelected;

    public:
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName);
        CreateEntityCmd(Project* project, uint32_t sceneId, std::string entityName, EntityCreationType type);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CREATEENTITYCMD_H */