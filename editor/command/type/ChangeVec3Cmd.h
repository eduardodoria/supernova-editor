#ifndef CHANGEVEC3CMD_H
#define CHANGEVEC3CMD_H

#include "command/Command.h"
#include "Scene.h"
#include "math/Vector3.h"
#include "ecs/Entity.h"
#include "component/Transform.h"
#include "Structure.h"


namespace Supernova::Editor{

    class ChangeVec3Cmd: public Command{

    private:
        Vector3& vector;
        Vector3 oldVector;
        Vector3 newVector;

        Transform* transform;

    public:
//        ChangeVec3Cmd(Scene* scene, Entity entity, ComponentType type, std::string property, Vector3 newVector);
        ChangeVec3Cmd(Vector3& originalVector, Vector3 newVector);
        ChangeVec3Cmd(Vector3& originalVector, Vector3 newVector, Transform* transform);

        virtual void execute();
        virtual void undo();

        virtual bool mergeWith(Command* olderCommand);
    };

}

#endif /* CHANGEVEC3CMD_H */