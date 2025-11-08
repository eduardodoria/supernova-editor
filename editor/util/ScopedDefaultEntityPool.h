#pragma once

#include "registry/EntityRegistry.h"

namespace Supernova::Editor {

    class ScopedDefaultEntityPool {
    public:
        ScopedDefaultEntityPool(EntityRegistry& r, EntityPool p)
            : reg(r), old(r.getDefaultEntityPool()) { reg.setDefaultEntityPool(p); }
        ~ScopedDefaultEntityPool() { reg.setDefaultEntityPool(old); }
    private:
        EntityRegistry& reg;
        EntityPool old;
    };

}