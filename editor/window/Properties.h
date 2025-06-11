#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Project.h"
#include "render/preview/MeshPreviewRender.h"

#include "imgui.h"

namespace Supernova::Editor{

    struct ShapeParameters{
        int geometryType = 0;
        float planeWidth = 1.0f, planeDepth = 1.0f;
        unsigned int planeTiles = 1;
        float boxWidth = 1.0f, boxHeight = 1.0f, boxDepth = 1.0f;
        unsigned int boxTiles = 1;
        float sphereRadius = 1.0f;
        unsigned int sphereSlices = 36, sphereStacks = 18;
        float cylinderBaseRadius = 1.0f, cylinderTopRadius = 1.0f, cylinderHeight = 2.0f;
        unsigned int cylinderSlices = 36, cylinderStacks = 18;
        float capsuleBaseRadius = 1.0f, capsuleTopRadius = 1.0f, capsuleHeight = 2.0f;
        unsigned int capsuleSlices = 36, capsuleStacks = 18;
        float torusRadius = 1.0f, torusRingRadius = 0.5f;
        unsigned int torusSides = 36, torusRings = 16;
    };

    class Properties{
    private:
        Project* project;
        Command* cmd;

        MaterialRender materialRender;
        MeshPreviewRender meshPreviewRender;

        // for drag and drop textures
        std::map<std::string, bool> hasTextureDrag;
        std::map<std::string, std::map<Entity, Texture>> originalTex;

        std::map<std::string, bool> materialButtonGroups;

        std::unordered_map<std::string, Texture> thumbnailTextures;

        const ImU32 textureLabel = IM_COL32(50, 50, 50, 255);

        // replace [number] with []
        std::string replaceNumberedBrackets(const std::string& input);
        Vector3 roundZero(const Vector3& val, const float threshold);

        bool compareVectorFloat(const float* a, const float* b, size_t elements, const float threshold);

        float getLabelSize(std::string label);

        void helpMarker(std::string desc);

        Texture* findThumbnail(const std::string& path);
        void drawImageWithBorderAndRounding(Texture* texture, const ImVec2& size, float rounding = 4.0f, ImU32 border_col = IM_COL32(0, 0, 0, 255), float border_thickness = 1.0f);
        void dragDropResources(ComponentType cpType, std::string id, SceneProject* sceneProject, std::vector<Entity> entities, int updateFlags);
        Texture getMaterialThumbnail(const Material& material);

        void updateMeshPreview(Scene* scene, Entity entity);
        void updateMeshShape(Entity entity, MeshSystem* meshSys, const ShapeParameters& shapeParams);

        void drawNinePatchesPreview(const ImageComponent& img, Texture* texture, Texture* thumbTexture, const ImVec2& size = ImVec2(0, 0));

        void beginTable(ComponentType cpType, float firstColSize, std::string nameAddon = "");
        void endTable();
        bool propertyHeader(std::string label, float secondColSize = -1, bool defChanged = false, bool child = false);
        bool propertyRow(ComponentType cpType, std::map<std::string, PropertyData> props, std::string id, std::string label, SceneProject* sceneProject, std::vector<Entity> entities, float stepSize = 0.1f, float secondColSize = -1, bool child = false, std::string help = "");

        void drawTransform(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawMeshComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawUIComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawUILayoutComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawImageComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawSpriteComponent(ComponentType cpType, std::map<std::string, PropertyData> props, SceneProject* sceneProject, std::vector<Entity> entities);

    public:
        Properties(Project* project);

        void show();
    };

}

#endif /* PROPERTIES_H */