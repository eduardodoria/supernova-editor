#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"

namespace Supernova::Editor {

    class ComponentToSharedCmd: public Command {
    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;
        ComponentType componentType;

        YAML::Node recovery;

    public:
        ComponentToSharedCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}