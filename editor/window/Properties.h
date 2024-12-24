#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Project.h"

namespace Supernova::Editor{

    class Properties{
    private:
        Project* project;

        // replace [number] with []
        std::string replaceNumberedBrackets(const std::string& input);
        Vector3 roundZero(const Vector3& val, const float threshold);

        float getMaxLabelSize(std::map<std::string, PropertyData> props, const std::string& include = "", const std::string& exclude = "");

        void beginTable(ComponentType cpType, float firstColSize, std::string nameAddon = "");
        void endTable();
        bool propertyHeader(std::string label, float secondColSize = -1, bool defChanged = false, bool child = false);
        void propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string name, Scene* scene, std::vector<Entity> entities, float secondColSize = -1, bool child = false);

        void drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities);
        void drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, Scene* scene, std::vector<Entity> entities);

    public:
        Properties(Project* project);

        void show();
    };

}

#endif /* PROPERTIES_H */