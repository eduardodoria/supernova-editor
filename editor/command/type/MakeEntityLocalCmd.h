#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class MakeEntityLocalCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;
        Entity parent;

        bool wasModified;

        NodeRecovery recovery;

    public:
        MakeEntityLocalCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}