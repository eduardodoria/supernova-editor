#ifndef DELETEENTITYCMD_H
#define DELETEENTITYCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    class DeleteEntityCmd: public Command{

    private:
        Project* project;
        uint32_t sceneId;
        Entity entity;

        Entity parent;
        std::string entityName;
        Signature signature;
        std::vector<Entity> lastSelected;
        
        Transform transform;
        MeshComponent mesh;

    public:
        DeleteEntityCmd(Project* project, uint32_t sceneId, Entity entity);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* DELETEENTITYCMD_H */