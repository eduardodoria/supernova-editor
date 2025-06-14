#pragma once

#include "command/Command.h"
#include "Project.h"
#include "util/ShapeParameters.h"
#include "yaml-cpp/yaml.h"

namespace Supernova::Editor{

    class MeshChangeCmd: public Command{

    private:
        YAML::Node oldMesh;
        YAML::Node newMesh;

        SceneProject* sceneProject;
        Entity entity;

        bool wasModified;

    public:
        MeshChangeCmd(SceneProject* sceneProject, Entity entity, MeshComponent mesh);

        virtual bool execute();
        virtual void undo();

        virtual bool mergeWith(Command* otherCommand);
    };

}