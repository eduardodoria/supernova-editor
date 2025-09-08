#pragma once

#include "command/Command.h"
#include "Project.h"
#include "Catalog.h"
#include "yaml-cpp/yaml.h"

namespace Supernova::Editor {
    class RemoveComponentCmd : public Command {
    private:
        Project* project;
        size_t sceneId;
        Entity entity;
        ComponentType componentType;

        YAML::Node oldComponent;

    public:
        RemoveComponentCmd(Project* project, size_t sceneId, Entity entity, ComponentType componentType);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };
}