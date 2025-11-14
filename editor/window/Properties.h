#pragma once

#include "Project.h"
#include "render/preview/MeshPreviewRender.h"
#include "render/preview/DirectionRender.h"
#include "util/ShapeParameters.h"
#include "window/dialog/ScriptCreateDialog.h"
#include "window/dialog/ComponentAddDialog.h"

#include "imgui.h"

namespace Supernova::Editor{

    enum class RowPropertyType{
        Bool,
        String,
        Float,
        Float_0_1,
        Vector2,
        Vector3,
        Vector4,
        Quat,
        Color3L,
        Color4L,
        Int,
        UInt,
        Material,
        Texture,
        HalfCone,
        UIntSlider,
        IntSlider,
        Direction,
        Enum,
        Custom,
        EntityPointer
    };

    struct EnumEntry {
        int value;
        const char* name;
    };

    struct RowSettings{
        float stepSize = 0.1f;
        float secondColSize = -1;
        bool child = false;
        std::string help = "";
        const char *format = "%.2f";
        std::vector<EnumEntry>* enumEntries = nullptr;
        std::vector<int>* sliderValues = nullptr;
        std::function<void()> onValueChanged = nullptr;
    };

    class Properties{
    private:
        Project* project;
        Command* cmd;

        bool finishProperty;

        std::set<std::string> usedPreviewIds;

        std::map<std::string, MaterialRender> materialRenders;
        std::map<std::string, DirectionRender> directionRenders;

        MeshPreviewRender shapePreviewRender;

        // for drag and drop textures
        std::map<std::string, bool> hasTextureDrag;
        std::map<std::string, std::map<Entity, Texture>> originalTex;

        std::map<std::string, bool> materialButtonGroups;

        std::unordered_map<std::string, Texture> thumbnailTextures;

        // For component menu
        char componentSearchBuffer[128] = "";
        int hoveredComponentIndex = -1;
        bool addComponentModalOpen = false;
        bool componentMenuJustOpened = false;

        // Dialogs
        ScriptCreateDialog scriptCreateDialog;
        ComponentAddDialog componentAddDialog;

        const ImU32 textureLabel = IM_COL32(50, 50, 50, 255);

        static RowPropertyType scriptPropertyTypeToRowPropertyType(ScriptPropertyType scriptType);

        // replace [number] with []
        std::string replaceNumberedBrackets(const std::string& input);
        Vector3 roundZero(const Vector3& val, const float threshold);

        bool compareVectorFloat(const float* a, const float* b, size_t elements, const float threshold);

        float getLabelSize(std::string label, bool addRotateIconSpace = true);

        void helpMarker(std::string desc);

        Texture* findThumbnail(const std::string& path);
        void drawImageWithBorderAndRounding(Texture* texture, const ImVec2& size, float rounding = 4.0f, ImU32 border_col = IM_COL32(0, 0, 0, 255), float border_thickness = 1.0f, bool flipY = false);
        void dragDropResources(ComponentType cpType, std::string id, SceneProject* sceneProject, std::vector<Entity> entities, ComponentType componentType);

        void handleComponentMenu(SceneProject* sceneProject, std::vector<Entity> entities, ComponentType cpType, bool isSharedGroup, bool isComponentOverridden, bool& headerOpen, bool readOnly);

        bool canAddComponent(SceneProject* sceneProject, Entity entity, ComponentType cpType);

        Texture getMaterialPreview(const Material& material, const std::string id);
        Texture getDirectionPreview(const Vector3& direction, const std::string id);

        void updateShapePreview(const ShapeParameters& shapeParams);
        void updateMeshShape(MeshComponent& meshComp, MeshSystem* meshSys, const ShapeParameters& shapeParams);

        void drawNinePatchesPreview(const ImageComponent& img, Texture* texture, Texture* thumbTexture, const ImVec2& size = ImVec2(0, 0));

        void beginTable(ComponentType cpType, float firstColSize, std::string nameAddon = "");
        void endTable();
        bool propertyHeader(std::string label, float secondColSize = -1, bool defChanged = false, bool child = false);
        bool propertyRow(RowPropertyType type, ComponentType cpType, std::string id, std::string label, SceneProject* sceneProject, std::vector<Entity> entities, RowSettings settings = RowSettings());

        void drawTransform(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawMeshComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawUIComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawUILayoutComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawImageComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawSpriteComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawLightComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);
        void drawScriptComponent(ComponentType cpType, SceneProject* sceneProject, std::vector<Entity> entities);

    public:
        Properties(Project* project);

        void show();
    };

}