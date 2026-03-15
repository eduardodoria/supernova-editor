#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class AddEntityToBundleCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;
        Entity parent;

        bool wasModified;

    public:
        AddEntityToBundleCmd(Project* project, uint32_t sceneId, Entity entity, Entity parent);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}
