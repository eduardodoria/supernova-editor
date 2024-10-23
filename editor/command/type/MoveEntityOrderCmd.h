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

        size_t originalIndex;

        size_t getIndex(std::vector<Entity>& entities, Entity entity);

    public:
        MoveEntityOrderCmd(Project* project, uint32_t sceneId, Entity source, Entity target, InsertionType type);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* MOVEENTITYORDERCMD_H */