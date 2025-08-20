#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <filesystem>
#include <vector>
#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class MarkEntitySharedCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;
        fs::path filepath;
        YAML::Node providedEntityNode; // Optional: if provided, use this instead of encoding

        // For undo
        bool wasModified;
        std::vector<Entity> lastSelected;
        YAML::Node savedEntityNode;
        bool wasSuccessful;

    public:
        MarkEntitySharedCmd(Project* project, uint32_t sceneId, Entity entity, const fs::path& filepath);
        MarkEntitySharedCmd(Project* project, uint32_t sceneId, Entity entity, const fs::path& filepath, const YAML::Node& entityNode);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}