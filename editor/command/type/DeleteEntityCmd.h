#pragma once

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

#include "yaml-cpp/yaml.h"

namespace Supernova::Editor{

    struct DeleteEntityData{
        Entity entity;

        size_t entityIndex;
        bool hasTransform = false;
        Entity parent = NULL_ENTITY;

        YAML::Node data;

        // Track if entity was part of a shared group
        NodeRecovery recoverySharedData;
    };

    class DeleteEntityCmd: public Command{

    friend class Project;
    friend class CreateEntityCmd;

    private:
        Project* project;
        uint32_t sceneId;

        std::vector<Entity> lastSelected;

        std::vector<DeleteEntityData> entities;

        bool wasModified;

        static void destroyEntity(EntityRegistry* registry, Entity entity, std::vector<Entity>& entities, Project* project = nullptr, uint32_t sceneId = NULL_PROJECT_SCENE);

    public:
        DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}