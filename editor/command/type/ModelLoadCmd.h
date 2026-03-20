#pragma once

#include "command/Command.h"
#include "Project.h"
#include "yaml-cpp/yaml.h"

namespace Supernova::Editor{

    class ModelLoadCmd: public Command{

    private:
        YAML::Node oldMesh;
        YAML::Node oldModel;

        Project* project;
        uint32_t sceneId;
        Entity entity;

        std::string modelPath;
        std::string oldModelPath;

        bool wasModified;

    public:
        ModelLoadCmd(Project* project, uint32_t sceneId, Entity entity, const std::string& modelPath);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}
