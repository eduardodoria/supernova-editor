#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"

namespace Supernova::Editor {
    class AddComponentCmd : public Command {
    private:
        Project* project;
        size_t sceneId;
        Entity entity;
        ComponentType componentType;

    public:
        AddComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };
}