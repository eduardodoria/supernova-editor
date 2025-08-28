#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Catalog.h"
#include "render/SceneRender.h"
#include "command/CommandHistory.h"
#include "render/preview/MaterialRender.h"
#include "util/EventBus.h"
#include "Conector.h"
#include "Generator.h"
#include "Configs.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Supernova::Editor{

    enum class SceneType{
        SCENE_3D,
        SCENE_2D,
        SCENE_UI
    };

    struct SceneProject{
        uint32_t id;
        std::string name;
        Scene* scene;
        SceneType sceneType;
        std::vector<Entity> entities;
        SceneRender* sceneRender;
        std::vector<Entity> selectedEntities;
        fs::path filepath;
        bool needUpdateRender;
        bool isModified;
    };

    struct SharedGroup {
        std::map<uint32_t, std::vector<Entity>> members; // sceneId → list of Entities (root + children)
        std::map<uint32_t, std::map<Entity, uint64_t>> overrides; // sceneId → entityId → bitmask of overridden ComponentTypes
        std::unique_ptr<EntityRegistry> registry;
        bool isModified = false;

        // Helper methods
        Entity getRootEntity(uint32_t sceneId) const {
            auto it = members.find(sceneId);
            if (it != members.end() && !it->second.empty()) {
                return it->second[0]; // First entity is always the root
            }
            return NULL_ENTITY;
        }

        const std::vector<Entity>& getAllEntities(uint32_t sceneId) const {
            static const std::vector<Entity> empty;
            auto it = members.find(sceneId);
            return (it != members.end()) ? it->second : empty;
        }

        // Get registry entity based on other scene entity
        Entity getRegistryEntity(uint32_t sceneId, Entity entity) const {
            auto it = members.find(sceneId);
            if (it != members.end()) {
                auto entIt = std::find(it->second.begin(), it->second.end(), entity);
                if (entIt != it->second.end()) {
                    size_t index = std::distance(it->second.begin(), entIt);
                    return index + NULL_ENTITY + 1; // Registry entities start from NULL_ENTITY + 1
                }
            }
            return NULL_ENTITY;
        }

        bool containsEntity(uint32_t sceneId, Entity entity) const {
            auto it = members.find(sceneId);
            if (it != members.end()) {
                return std::find(it->second.begin(), it->second.end(), entity) != it->second.end();
            }
            return false;
        }

        // New methods for override management using ComponentType bit operations
        void setComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
            uint64_t bit = 1ULL << static_cast<int>(componentType);
            overrides[sceneId][entity] |= bit;
        }

        void clearComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
            auto sceneIt = overrides.find(sceneId);
            if (sceneIt != overrides.end()) {
                auto entityIt = sceneIt->second.find(entity);
                if (entityIt != sceneIt->second.end()) {
                    uint64_t bit = 1ULL << static_cast<int>(componentType);
                    entityIt->second &= ~bit;
                    // Remove entity entry if no overrides remain
                    if (entityIt->second == 0) {
                        sceneIt->second.erase(entityIt);
                    }
                    // Remove scene entry if no entities have overrides
                    if (sceneIt->second.empty()) {
                        overrides.erase(sceneIt);
                    }
                }
            }
        }

        bool hasComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) const {
            auto sceneIt = overrides.find(sceneId);
            if (sceneIt != overrides.end()) {
                auto entityIt = sceneIt->second.find(entity);
                if (entityIt != sceneIt->second.end()) {
                    uint64_t bit = 1ULL << static_cast<int>(componentType);
                    return (entityIt->second & bit) != 0;
                }
            }
            return false;
        }

        uint64_t getEntityOverrides(uint32_t sceneId, Entity entity) const {
            auto sceneIt = overrides.find(sceneId);
            if (sceneIt != overrides.end()) {
                auto entityIt = sceneIt->second.find(entity);
                if (entityIt != sceneIt->second.end()) {
                    return entityIt->second;
                }
            }
            return 0; // No overrides
        }

        void clearAllOverrides(uint32_t sceneId, Entity entity) {
            auto sceneIt = overrides.find(sceneId);
            if (sceneIt != overrides.end()) {
                sceneIt->second.erase(entity);
                if (sceneIt->second.empty()) {
                    overrides.erase(sceneIt);
                }
            }
        }

        void clearAllSceneOverrides(uint32_t sceneId) {
            overrides.erase(sceneId);
        }

        // Helper method to check if any components are overridden for an entity
        bool hasAnyOverrides(uint32_t sceneId, Entity entity) const {
            return getEntityOverrides(sceneId, entity) != 0;
        }

        // Helper method to get all overridden component types for an entity
        std::vector<ComponentType> getOverriddenComponents(uint32_t sceneId, Entity entity) const {
            std::vector<ComponentType> result;
            uint64_t overrideMask = getEntityOverrides(sceneId, entity);

            for (int i = 0; i < 64; ++i) {
                if (overrideMask & (1ULL << i)) {
                    result.push_back(static_cast<ComponentType>(i));
                }
            }

            return result;
        }
    };

    struct NodeRecoveryEntry {
        YAML::Node node;
        std::vector<size_t> indicesNotShared;
    };

    using NodeRecovery = std::map<uint32_t, NodeRecoveryEntry>;

    class Project{
    private:

        static EventBus globalEventBus;

        Conector conector;
        Generator generator;

        std::string name;

        unsigned int windowWidth;
        unsigned int windowHeight;

        uint32_t nextSceneId;

        std::vector<SceneProject> scenes;
        uint32_t selectedScene;

        std::filesystem::path projectPath;
        bool resourcesFocused;

        std::map<std::filesystem::path, SharedGroup> sharedGroups;

        template<typename T>
        T* findScene(uint32_t sceneId) const;

        void setupSharedGroupEventSubscriptions(const std::filesystem::path& filepath);

        Entity createNewEntity(uint32_t sceneId, std::string entityName);
        bool createNewComponent(uint32_t sceneId, Entity entity, ComponentType component);
        void deleteSceneProject(SceneProject* sceneProject);
        void resetConfigs();

        size_t countEntitiesInBranch(const YAML::Node& entityNode);
        void insertNewChild(YAML::Node& node, YAML::Node child, size_t index);
        std::vector<size_t> mergeEntityNodesImpl(YAML::Node& loadedNode, const YAML::Node& extendNode, size_t& index);

    public:
        Project();

        std::string getName() const;
        void setName(std::string name);

        void setWindowSize(unsigned int width, unsigned int height);
        unsigned int getWindowWidth() const;
        unsigned int getWindowHeight() const;

        bool createTempProject(std::string projectName, bool deleteIfExists = false);
        bool saveProjectToPath(const std::filesystem::path& path);
        bool saveProject(bool userCalled = false, std::function<void()> callback = nullptr);
        bool openProject();

        bool loadProject(const std::filesystem::path path);

        void saveScene(uint32_t sceneId);
        void saveSceneToPath(uint32_t sceneId, const std::filesystem::path& path);
        void saveAllScenes();
        void saveLastSelectedScene();

        uint32_t createNewScene(std::string sceneName, SceneType type);
        void openScene(fs::path filepath);
        void closeScene(uint32_t sceneId);

        Entity findObjectByRay(uint32_t sceneId, float x, float y);

        bool selectObjectByRay(uint32_t sceneId, float x, float y, bool shiftPressed);
        bool selectObjectsByRect(uint32_t sceneId, Vector2 start, Vector2 end);

        std::vector<SceneProject>& getScenes();
        SceneProject* getScene(uint32_t sceneId);
        const SceneProject* getScene(uint32_t sceneId) const;
        SceneProject* getSelectedScene();
        const SceneProject* getSelectedScene() const;

        void setNextSceneId(uint32_t nextSceneId);
        uint32_t getNextSceneId() const;

        void setSelectedSceneId(uint32_t selectedScene);
        uint32_t getSelectedSceneId() const;

        bool isTempProject() const;
        bool isTempUnsavedProject() const;
        std::filesystem::path getProjectPath() const;

        fs::path getThumbsDir() const;
        fs::path getThumbnailPath(const fs::path& originalPath) const;

        std::vector<Entity> getEntities(uint32_t sceneId) const;

        void replaceSelectedEntities(uint32_t sceneId, std::vector<Entity> selectedEntities);
        void setSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        void addSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        bool isSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        void clearSelectedEntities(uint32_t sceneId);
        std::vector<Entity> getSelectedEntities(uint32_t sceneId) const;
        bool hasSelectedEntities(uint32_t sceneId) const;

        bool hasSelectedSceneUnsavedChanges() const;
        bool hasScenesUnsavedChanges() const;

        static EventBus& getEventBus();

        bool markEntityShared(uint32_t sceneId, Entity entity, fs::path filepath, YAML::Node entityNode);
        bool removeSharedGroup(const std::filesystem::path& filepath);

        std::vector<Entity> importSharedEntity(SceneProject* sceneProject, const std::filesystem::path& filepath, Entity parent = NULL_ENTITY, bool needSaveScene = true, YAML::Node extendNode = YAML::Node());
        bool unimportSharedEntity(uint32_t sceneId, const std::filesystem::path& filepath, const std::vector<Entity>& entities, bool destroyEntities = true);

        bool addEntityToSharedGroup(uint32_t sceneId, NodeRecovery entityData, Entity parent, const std::filesystem::path& filepath, bool createItself = true);
        NodeRecovery removeEntityFromSharedGroup(uint32_t sceneId, Entity entity, const std::filesystem::path& filepath, bool destroyItself = true);

        void saveSharedGroupToDisk(const std::filesystem::path& filepath);

        SharedGroup* getSharedGroup(const std::filesystem::path& filepath);
        const SharedGroup* getSharedGroup(const std::filesystem::path& filepath) const;
        std::filesystem::path findGroupPathFor(uint32_t sceneId, Entity e) const;

        std::vector<size_t> mergeEntityNodes(YAML::Node& loadedNode, const YAML::Node& extendNode);

        void collectEntities(const YAML::Node& entityNode, std::vector<Entity>& allEntities, std::vector<Entity>& sharedEntities);

        void build();

        void debugSceneHierarchy();
    };

}

#endif /* PROJECT_H */