#ifndef MOVEENTITYORDERCMD_H
#define MOVEENTITYORDERCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    enum class InsertionType{
        BEFORE,
        AFTER,
        IN
    };

    class MoveEntityOrderCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;

        Entity source;
        Entity target;
        InsertionType type;

        size_t oldIndex;
        size_t oldTransformIndex;
        Entity oldParent;

        void sortEntitiesByTransformOrder(std::vector<Entity>& entities, Scene* scene);

    public:
        MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* MOVEENTITYORDERCMD_H */