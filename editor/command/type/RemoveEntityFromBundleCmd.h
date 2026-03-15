#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class RemoveEntityFromBundleCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;
        Entity parent;

        bool wasModified;

        NodeRecovery recovery;

    public:
        RemoveEntityFromBundleCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}
