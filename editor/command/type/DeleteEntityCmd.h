#ifndef DELETEENTITYCMD_H
#define DELETEENTITYCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

#include "yaml-cpp/yaml.h"

namespace Supernova::Editor{

    struct DeleteEntityData{
        Entity entity;

        size_t transformIndex;
        size_t entityIndex;
        Entity parent = NULL_ENTITY;

        YAML::Node data;
    };

    class DeleteEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;

        std::vector<Entity> lastSelected;

        std::vector<DeleteEntityData> entities;

        bool wasModified;

        void collectEntities(const YAML::Node& entityNode, std::vector<Entity>& entities);

    public:
        DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* DELETEENTITYCMD_H */