#pragma once

#include "yaml-cpp/yaml.h"

#include "Scene.h"
#include "ecs/Entity.h"

namespace Supernova::Editor{

    class Stream{
    private:

    public:

        static void serializeEntity(YAML::Emitter& out, Entity entity, Scene* scene);

    };

}