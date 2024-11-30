#ifndef LAYOUTPROPERTIES_H
#define LAYOUTPROPERTIES_H

#include "Project.h"

namespace Supernova::Editor{

    class Properties{
    private:
        Project* project;

        void drawPropertiesTable(ComponentType cpType, std::vector<PropertyData> props, Scene* scene, Entity entity, std::string tableNameAddon = "", float inputWidth = -1);

    public:
        Properties(Project* project);

        void show();
    };

}

#endif /* LAYOUTPROPERTIES_H */