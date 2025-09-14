#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Catalog.h"
#include "render/SceneRender.h"
#include "command/CommandHistory.h"
#include "render/preview/MaterialRender.h"
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

    enum class InsertionType{
        BEFORE,
        AFTER,
        IN
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

    struct MergeResult {
        bool isShared; // Whether the entity was already part of a shared group
        uint64_t overrides; // Bitmask of overridden ComponentTypes for each entity
    };

    struct SharedGroup {

        struct EntityMember {
            Entity localEntity;    // Entity in the scene
            Entity registryEntity; // Corresponding entity in the shared registry
        };

        struct Instance {
            uint32_t instanceId;
            std::vector<EntityMember> members;
            std::map<Entity, uint64_t> overrides; // entityId → bitmask of overridden ComponentTypes
        };

        // sceneId → list of instances
        std::map<uint32_t, std::vector<Instance>> instances;

        std::unique_ptr<EntityRegistry> registry;
        std::vector<Entity> registryEntities;
        bool isModified = false;

        uint32_t nextInstanceId = 1;

        // Helper methods
        Entity getRootEntity(uint32_t sceneId, uint32_t instanceId) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    if (instance.instanceId == instanceId && !instance.members.empty()) {
                        return instance.members[0].localEntity;
                    }
                }
            }
            return NULL_ENTITY;
        }

        // Get all local entities for a specific instance
        std::vector<Entity> getAllEntities(uint32_t sceneId, uint32_t instanceId) const {
            auto it = instances.find(sceneId);
            if (it == instances.end()) return {};

            for (const auto& instance : it->second) {
                if (instance.instanceId == instanceId) {
                    std::vector<Entity> result;
                    result.reserve(instance.members.size());
                    for (const auto& member : instance.members) {
                        result.push_back(member.localEntity);
                    }
                    return result;
                }
            }
            return {};
        }

        // Get all instances for a scene
        std::vector<uint32_t> getInstanceIds(uint32_t sceneId) const {
            std::vector<uint32_t> result;
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    result.push_back(instance.instanceId);
                }
            }
            return result;
        }

        // Get the instance ID for a given entity
        uint32_t getInstanceId(uint32_t sceneId, Entity entity) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        if (member.localEntity == entity) {
                            return instance.instanceId;
                        }
                    }
                }
            }
            return 0; // Invalid instance ID
        }

        // Get registry entity based on local entity (searches all instances)
        Entity getRegistryEntity(uint32_t sceneId, Entity entity) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        if (member.localEntity == entity) {
                            return member.registryEntity;
                        }
                    }
                }
            }
            return NULL_ENTITY;
        }

        // Get local entity based on registry entity for a specific instance
        Entity getLocalEntity(uint32_t sceneId, uint32_t instanceId, Entity registryEntity) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    if (instance.instanceId == instanceId) {
                        for (const auto& member : instance.members) {
                            if (member.registryEntity == registryEntity) {
                                return member.localEntity;
                            }
                        }
                    }
                }
            }
            return NULL_ENTITY;
        }

        // Check if entity exists in any instance
        bool containsEntity(uint32_t sceneId, Entity entity) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        if (member.localEntity == entity) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        // Get instance pointer by entity (non-const version)
        Instance* getInstance(uint32_t sceneId, Entity entity) {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        if (member.localEntity == entity) {
                            return &instance;
                        }
                    }
                }
            }
            return nullptr;
        }

        // Get instance pointer by entity (const version)
        const Instance* getInstance(uint32_t sceneId, Entity entity) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        if (member.localEntity == entity) {
                            return &instance;
                        }
                    }
                }
            }
            return nullptr;
        }

        // Get instance pointer by ID
        Instance* getInstanceById(uint32_t sceneId, uint32_t instanceId) {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (auto& instance : it->second) {
                    if (instance.instanceId == instanceId) {
                        return &instance;
                    }
                }
            }
            return nullptr;
        }

        const Instance* getInstanceById(uint32_t sceneId, uint32_t instanceId) const {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    if (instance.instanceId == instanceId) {
                        return &instance;
                    }
                }
            }
            return nullptr;
        }

        // Remove a specific instance
        void removeInstance(uint32_t sceneId, uint32_t instanceId) {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                auto& sceneInstances = it->second;
                sceneInstances.erase(
                    std::remove_if(sceneInstances.begin(), sceneInstances.end(),
                        [instanceId](const Instance& inst) { return inst.instanceId == instanceId; }),
                    sceneInstances.end()
                );

                if (sceneInstances.empty()) {
                    instances.erase(it);
                }
            }
        }

        // Override management methods
        void setComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) {
            Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                uint64_t bit = 1ULL << static_cast<int>(componentType);
                instance->overrides[entity] |= bit;
            }
        }

        void setComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                setComponentOverride(sceneId, instanceId, entity, componentType);
            }
        }

        void clearComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) {
            Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                auto entityIt = instance->overrides.find(entity);
                if (entityIt != instance->overrides.end()) {
                    uint64_t bit = 1ULL << static_cast<int>(componentType);
                    entityIt->second &= ~bit;
                    // Remove entity entry if no overrides remain
                    if (entityIt->second == 0) {
                        instance->overrides.erase(entityIt);
                    }
                }
            }
        }

        void clearComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                clearComponentOverride(sceneId, instanceId, entity, componentType);
            }
        }

        bool hasComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) const {
            const Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                auto entityIt = instance->overrides.find(entity);
                if (entityIt != instance->overrides.end()) {
                    uint64_t bit = 1ULL << static_cast<int>(componentType);
                    return (entityIt->second & bit) != 0;
                }
            }
            return false;
        }

        bool hasComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) const {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                return hasComponentOverride(sceneId, instanceId, entity, componentType);
            }
            return false;
        }

        uint64_t getEntityOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
            const Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                auto entityIt = instance->overrides.find(entity);
                if (entityIt != instance->overrides.end()) {
                    return entityIt->second;
                }
            }
            return 0; // No overrides
        }

        uint64_t getEntityOverrides(uint32_t sceneId, Entity entity) const {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                return getEntityOverrides(sceneId, instanceId, entity);
            }
            return 0;
        }

        void clearAllOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) {
            Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                instance->overrides.erase(entity);
            }
        }

        void clearAllOverrides(uint32_t sceneId, Entity entity) {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                clearAllOverrides(sceneId, instanceId, entity);
            }
        }

        void clearAllInstanceOverrides(uint32_t sceneId, uint32_t instanceId) {
            Instance* instance = getInstanceById(sceneId, instanceId);
            if (instance) {
                instance->overrides.clear();
            }
        }

        void clearAllSceneOverrides(uint32_t sceneId) {
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (auto& instance : it->second) {
                    instance.overrides.clear();
                }
            }
        }

        // Helper method to check if any components are overridden for an entity
        bool hasAnyOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
            return getEntityOverrides(sceneId, instanceId, entity) != 0;
        }

        bool hasAnyOverrides(uint32_t sceneId, Entity entity) const {
            return getEntityOverrides(sceneId, entity) != 0;
        }

        // Helper method to get all overridden component types for an entity
        std::vector<ComponentType> getOverriddenComponents(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
            std::vector<ComponentType> result;
            uint64_t overrideMask = getEntityOverrides(sceneId, instanceId, entity);

            for (int i = 0; i < 64; ++i) {
                if (overrideMask & (1ULL << i)) {
                    result.push_back(static_cast<ComponentType>(i));
                }
            }

            return result;
        }

        std::vector<ComponentType> getOverriddenComponents(uint32_t sceneId, Entity entity) const {
            uint32_t instanceId = getInstanceId(sceneId, entity);
            if (instanceId != 0) {
                return getOverriddenComponents(sceneId, instanceId, entity);
            }
            return {};
        }

        // Check if scene has any instances
        bool hasInstances(uint32_t sceneId) const {
            auto it = instances.find(sceneId);
            return it != instances.end() && !it->second.empty();
        }

        // Get total number of instances across all scenes
        size_t getTotalInstanceCount() const {
            size_t count = 0;
            for (const auto& [sceneId, sceneInstances] : instances) {
                count += sceneInstances.size();
            }
            return count;
        }

        // Get all entities from all instances in a scene
        std::vector<Entity> getAllSceneEntities(uint32_t sceneId) const {
            std::vector<Entity> result;
            auto it = instances.find(sceneId);
            if (it != instances.end()) {
                for (const auto& instance : it->second) {
                    for (const auto& member : instance.members) {
                        result.push_back(member.localEntity);
                    }
                }
            }
            return result;
        }
    };

    struct NodeRecoveryEntry {
        YAML::Node node;
        std::vector<MergeResult> mergeResults;
        size_t transformIndex;
    };

    using NodeRecovery = std::map<std::string, NodeRecoveryEntry>;

    struct SharedMoveRecoveryEntry {
        Entity oldParent;
        size_t oldIndex;
        size_t hasTransform;
    };

    using SharedMoveRecovery = std::map<std::string, SharedMoveRecoveryEntry>;

    struct ComponentRecoveryEntry {
        Entity entity;
        YAML::Node node;
    };

    using ComponentRecovery = std::map<std::string, ComponentRecoveryEntry>;

    class Project{
    private:

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

        Entity createNewEntity(uint32_t sceneId, std::string entityName);
        bool createNewComponent(uint32_t sceneId, Entity entity, ComponentType component);
        void deleteSceneProject(SceneProject* sceneProject);
        void resetConfigs();

        size_t countEntitiesInBranch(const YAML::Node& entityNode);
        void insertNewChild(YAML::Node& node, YAML::Node child, size_t index);

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

        static size_t getTransformIndex(EntityRegistry* registry, Entity entity);
        static void sortEntitiesByTransformOrder(EntityRegistry* registry, std::vector<Entity>& entities);

        static void addEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, YAML::Node componentNode = YAML::Node());
        static YAML::Node removeEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, bool encodeComponent = false);

        //=== Shared Entities part ===

        bool markEntityShared(uint32_t sceneId, Entity entity, fs::path filepath, YAML::Node entityNode);
        bool removeSharedGroup(const std::filesystem::path& filepath);

        std::vector<Entity> importSharedEntity(SceneProject* sceneProject, const std::filesystem::path& filepath, Entity parent = NULL_ENTITY, bool needSaveScene = true, YAML::Node extendNode = YAML::Node());
        bool unimportSharedEntity(uint32_t sceneId, const std::filesystem::path& filepath, const std::vector<Entity>& entities, bool destroyEntities = true);

        bool addEntityToSharedGroup(uint32_t sceneId, Entity entity, Entity parent, bool createItself = true);
        bool addEntityToSharedGroup(uint32_t sceneId, const NodeRecovery& recoveryData, Entity parent, uint32_t instanceId = 0, bool createItself = true);
        NodeRecovery removeEntityFromSharedGroup(uint32_t sceneId, Entity entity, bool destroyItself = true);

        SharedMoveRecovery moveEntityFromSharedGroup(uint32_t sceneId, Entity entity, Entity target, InsertionType type, bool moveItself = true);
        bool undoMoveEntityInSharedGroup(uint32_t sceneId, Entity entity, Entity target, const SharedMoveRecovery& recovery, bool moveItself = true);

        bool addComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, bool addToItself = true);
        bool addComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, const ComponentRecovery& recovery, bool addToItself = true);
        ComponentRecovery removeComponentToSharedGroup(uint32_t sceneId, Entity entity, ComponentType componentType, bool encodeComponent = true, bool removeToItself = true);

        void saveSharedGroupToDisk(const std::filesystem::path& filepath);

        SharedGroup* getSharedGroup(const std::filesystem::path& filepath);
        const SharedGroup* getSharedGroup(const std::filesystem::path& filepath) const;
        std::filesystem::path findGroupPathFor(uint32_t sceneId, Entity entity) const;
        bool isEntityShared(uint32_t sceneId, Entity entity) const;

        std::vector<MergeResult> mergeEntityNodes(const YAML::Node& extendNode, YAML::Node& outputNode);

        YAML::Node clearEntitiesNode(YAML::Node node);
        YAML::Node changeEntitiesNode(Entity& firstEntity, YAML::Node node);

        void collectEntities(const YAML::Node& entityNode, std::vector<Entity>& allEntities);
        void collectEntities(const YAML::Node& entityNode, std::vector<Entity>& allEntities, std::vector<Entity>& sharedEntities);

        bool sharedGroupComponentChanged(uint32_t sceneId, Entity entity, ComponentType componentType, std::vector<std::string> properties, bool renderAllScenes = true);

        //=== end Shared Entities part ===

        void build();

        void debugSceneHierarchy();
    };

}

#endif /* PROJECT_H */