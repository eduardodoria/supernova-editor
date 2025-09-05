#ifndef MOVEENTITYORDERCMD_H
#define MOVEENTITYORDERCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class MoveEntityOrderCmd: public Command{

    friend class Project;

    private:
        Project* project;
        uint32_t sceneId;

        Entity source;
        Entity target;
        InsertionType type;

        size_t oldIndex;
        bool hasTransform;
        Entity oldParent;

        SharedMoveRecovery sharedMoveRecovery;

        bool wasModified;

        static bool moveEntityOrderByTarget(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, bool& hasTransform);
        static void moveEntityOrderByIndex(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, Entity oldParent, size_t oldIndex, bool hasTransform);
        static void moveEntityOrderByTransform(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t transformIndex, bool enableMove = true);

    public:
        MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* MOVEENTITYORDERCMD_H */