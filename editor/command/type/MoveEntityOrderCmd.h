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
        size_t oldTransformIndex;
        Entity oldParent;

        SharedMoveRecovery sharedMoveRecovery;

        bool wasModified;

        static void sortEntitiesByTransformOrder(std::vector<Entity>& entities, EntityRegistry* registry);

        static bool changeEntityOrder(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, size_t& oldTransformIndex);
        static void undoEntityOrder(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, Entity oldParent, size_t oldIndex, size_t oldTransformIndex);

    public:
        MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* MOVEENTITYORDERCMD_H */