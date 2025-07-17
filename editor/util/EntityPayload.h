#pragma once
#include <cstddef>
#include "ecs/Entity.h"

namespace Supernova {

    struct EntityPayload {
        Entity entity;
        Entity parent;
        size_t order;
        bool hasTransform;
    }; // YAML string follows

}