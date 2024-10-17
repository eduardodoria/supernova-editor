#ifndef CHANGEVEC3CMD_H
#define CHANGEVEC3CMD_H

#include "command/Command.h"
#include "math/Vector3.h"


namespace Supernova::Editor{

    class ChangeVec3Cmd: public Command{

    private:
        Vector3& vector;
        Vector3 oldVector;
        Vector3 newVector;

    public:
        ChangeVec3Cmd(Vector3& originalVector, Vector3& newVector);

        virtual void execute();
        virtual void undo();
    };

}

#endif /* CHANGEVEC3CMD_H */