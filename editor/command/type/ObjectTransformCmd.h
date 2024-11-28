#ifndef CHANGEOBJTRANSFCMD_H
#define CHANGEOBJTRANSFCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    struct CommandTransform{
        Vector3 newPosition;
        Quaternion newRotation;
        Vector3 newScale;

        Vector3 oldPosition;
        Quaternion oldRotation;
        Vector3 oldScale;
    };

    class ObjectTransformCmd: public Command{

    private:
        Scene* scene;

        std::map<Entity, CommandTransform> props;

    public:
        ObjectTransformCmd(Scene* scene, Entity entity, Matrix4 localMatrix);
        ObjectTransformCmd(Scene* scene, Entity entity, Vector3 position, Quaternion rotation, Vector3 scale);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CHANGEOBJTRANSFCMD_H */