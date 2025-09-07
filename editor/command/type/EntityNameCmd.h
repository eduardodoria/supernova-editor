#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class EntityNameCmd: public Command{

    private:
        std::string oldName;
        std::string newName;

        SceneProject* sceneProject;
        Entity entity;

        bool wasModified;

    public:
        EntityNameCmd(SceneProject* sceneProject, Entity entity, std::string name);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}