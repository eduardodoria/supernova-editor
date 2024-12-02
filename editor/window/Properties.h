#ifndef LAYOUTPROPERTIES_H
#define LAYOUTPROPERTIES_H

#include "Project.h"

namespace Supernova::Editor{

    class Properties{
    private:
        Project* project;

        float getMaxLabelSize(std::map<std::string, PropertyData> props, const std::string& include = "", const std::string& exclude = "");

        void beginTable(ComponentType cpType, float firstColSize, std::string nameAddon = "");
        void endTable();
        void propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string name, Scene* scene, Entity entity, float secondColSize = -1);

        void drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, Entity entity);
        void drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, Entity entity);

    public:
        Properties(Project* project);

        void show();
    };

}

#endif /* LAYOUTPROPERTIES_H */