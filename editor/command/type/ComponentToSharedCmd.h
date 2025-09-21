#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"

namespace Supernova::Editor {

    struct ComponentToSharedData {
        Entity entity;
        YAML::Node recovery;
    };

    class ComponentToSharedCmd: public Command {
    private:
        Project* project;
        uint32_t sceneId;
        ComponentType componentType;

        std::vector<ComponentToSharedData> entities;

        bool wasModified;

    public:
        ComponentToSharedCmd(Project* project, uint32_t sceneId, Entity entity, ComponentType componentType);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}