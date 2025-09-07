#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>
#include <filesystem>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

namespace Supernova::Editor{

    class ImportSharedEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        fs::path filepath;
        Entity parent;
        bool needSaveScene;
        YAML::Node extendNode;

        std::vector<Entity> importedEntities;
        std::vector<Entity> lastSelected;
        bool wasModified;

    public:
        ImportSharedEntityCmd(Project* project, uint32_t sceneId, const fs::path& filepath, Entity parent = NULL_ENTITY, bool needSaveScene = true);
        ImportSharedEntityCmd(Project* project, uint32_t sceneId, const fs::path& filepath, Entity parent, bool needSaveScene, YAML::Node extendNode);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;

        std::vector<Entity> getImportedEntities() const;
    };

}