#ifndef CHANGEOBJTRANSFCMD_H
#define CHANGEOBJTRANSFCMD_H

#include "command/Command.h"
#include "Project.h"
#include <cstdint>
#include <string>

namespace Supernova::Editor{

    struct TransformCmdValue{
        Vector3 newPosition;
        Quaternion newRotation;
        Vector3 newScale;

        Vector3 oldPosition;
        Quaternion oldRotation;
        Vector3 oldScale;
    };

    class ObjectTransformCmd: public Command{

    private:
        SceneProject* sceneProject;

        std::map<Entity, TransformCmdValue> props;

    public:
        ObjectTransformCmd(SceneProject* sceneProject, Entity entity, Matrix4 localMatrix);
        ObjectTransformCmd(SceneProject* sceneProject, Entity entity, Vector3 position, Quaternion rotation, Vector3 scale);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}

#endif /* CHANGEOBJTRANSFCMD_H */