#ifndef LAYOUTPROPERTIES_H
#define LAYOUTPROPERTIES_H

#include "Project.h"

namespace Supernova::Editor{

    class Properties{
    private:
        Project* project;

    public:
        Properties(Project* project);

        void show();
    };

}

#endif /* LAYOUTPROPERTIES_H */