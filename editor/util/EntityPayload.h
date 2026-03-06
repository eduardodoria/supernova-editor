#pragma once
#include <cstddef>
#include <cstdint>
#include "ecs/Entity.h"

namespace Supernova {

    struct EntityPayload {
        Entity entity;
        Entity parent;
        size_t order;
        bool hasTransform;
    }; // YAML string follows

    struct MaterialPayload {
        uint32_t magic;
        uint32_t sceneId;
        Entity entity;
        unsigned int submeshIndex;
    }; // YAML string follows

}