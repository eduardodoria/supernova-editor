#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Project.h"

#include "imgui.h"

namespace Supernova::Editor{

    class Properties{
    private:
        Project* project;

        const ImU32 textureLabel = IM_COL32(50, 50, 50, 255);

        // replace [number] with []
        std::string replaceNumberedBrackets(const std::string& input);
        Vector3 roundZero(const Vector3& val, const float threshold);

        float getMaxLabelSize(std::map<std::string, PropertyData> props, const std::vector<std::string>& includes = {}, const std::vector<std::string>& excludes = {});
        std::vector<std::string> getStringsFromPayload(const ImGuiPayload* payload);

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