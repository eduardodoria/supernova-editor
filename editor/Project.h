#pragma once

#include "Scene.h"
#include "Catalog.h"
#include "render/SceneRender.h"
#include "command/CommandHistory.h"
#include "render/preview/MaterialRender.h"
#include "Conector.h"
#include "Generator.h"
#include "Configs.h"
#include "util/SharedGroup.h"
#include "util/ScriptParser.h"
#include "util/ScopedDefaultEntityPool.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

namespace Supernova::Editor{

    enum class SceneType{
        SCENE_3D,
        SCENE_2D,
        SCENE_UI
    };

    enum class ScenePlayState {
        STOPPED,
        PLAYING,
        PAUSED,
        CANCELLING
    };

    enum class InsertionType{
        BEFORE,
        AFTER,
        INTO
    };

    struct SceneProject{
        uint32_t id = NULL_PROJECT_SCENE;
        std::string name = "Unknown";
        Scene* scene = nullptr;
        SceneType sceneType;
        std::vector<Entity> entities;
        Camera* defaultCamera = nullptr;
        Entity mainCamera = NULL_ENTITY;
        SceneRender* sceneRender = nullptr;
        std::vector<Entity> selectedEntities;
        fs::path filepath;
        bool needUpdateRender = true;
        bool isModified = false;
        bool isVisible = false;
        bool opened = true;
        ScenePlayState playState = ScenePlayState::STOPPED;
        YAML::Node playStateSnapshot;
        std::vector<uint32_t> childScenes;
    };

    struct MergeResult {
        bool isShared; // Whether the entity was already part of a shared group
        uint64_t overrides; // Bitmask of overridden ComponentTypes for each entity
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

        struct PlayRuntimeScene {
            uint32_t sourceSceneId = NULL_PROJECT_SCENE;
            SceneProject* runtime = nullptr; // Scene used by the Engine while playing
            bool ownedRuntime = false;       // true when runtime was cloned and must be deleted
        };

        struct PlaySession {
            uint32_t mainSceneId = NULL_PROJECT_SCENE;
            std::vector<PlayRuntimeScene> runtimeScenes;
            std::atomic<bool> cancelled{false};
            std::atomic<bool> startupThreadDone{false};  // Set when connect thread exits
            std::atomic<bool> startupSucceeded{false};   // True only if finalizeStart was called
        };

        mutable std::mutex playSessionMutex;
        std::shared_ptr<PlaySession> activePlaySession;

        SceneProject* createRuntimeCloneFromSource(const SceneProject* source);
        void cleanupPlaySession(const std::shared_ptr<PlaySession>& session);

        SceneRender* createSceneRender(SceneType type, Scene* scene) const;
        Camera* createDefaultCamera(SceneType type, Scene* scene) const;
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

        std::vector<Editor::ScriptSource> collectCppScriptSourceFiles(const std::vector<PlayRuntimeScene>& runtimeScenes) const;

        void pauseEngineScene(SceneProject* sceneProject, bool pause);

        void copyEngineApiToProject();

        void initializeLuaScripts(SceneProject* sceneProject);
        void cleanupLuaScripts(SceneProject* sceneProject);

        void finalizeStart(SceneProject* mainSceneProject, const std::vector<PlayRuntimeScene>& runtimeScenes);
        void finalizeStop(SceneProject* mainSceneProject, const std::vector<PlayRuntimeScene>& runtimeScenes);

        void collectInvolvedScenes(uint32_t sceneId, std::vector<uint32_t>& involvedSceneIds);

        uint32_t createNewSceneInternal(std::string sceneName, SceneType type, uint32_t previousSceneId);
        void openSceneInternal(fs::path filepath, uint32_t sceneToClose);

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

        void checkUnsavedAndExecute(uint32_t sceneId, std::function<void()> action);

        void saveScene(uint32_t sceneId);
        void saveSceneToPath(uint32_t sceneId, const std::filesystem::path& path);
        void saveAllScenes();
        void saveLastSelectedScene();

        uint32_t createNewScene(std::string sceneName, SceneType type);
        void loadScene(fs::path filepath, bool opened, bool isNewScene = true);
        void openScene(fs::path filepath, bool closePrevious = true);
        void closeScene(uint32_t sceneId);

        void addChildScene(uint32_t sceneId, uint32_t childSceneId);
        void removeChildScene(uint32_t sceneId, uint32_t childSceneId);
        bool hasChildScene(uint32_t sceneId, uint32_t childSceneId) const;
        std::vector<uint32_t> getChildScenes(uint32_t sceneId) const;

        Entity findObjectByRay(uint32_t sceneId, float x, float y);

        bool selectObjectByRay(uint32_t sceneId, float x, float y, bool shiftPressed);
        bool selectObjectsByRect(uint32_t sceneId, Vector2 start, Vector2 end);

        std::vector<SceneProject>& getScenes();
        const std::vector<SceneProject>& getScenes() const;
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
        std::filesystem::path getProjectInternalPath() const;

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

        void updateAllScriptsProperties(uint32_t sceneId);
        void updateScriptProperties(SceneProject* sceneProject, Entity entity, std::vector<ScriptEntry>& scripts);

        void resolveEntityRef(EntityRef& ref, SceneProject* sceneProject, Entity entity);
        void resolveEntityRefs(SceneProject* sceneProject);
        void resolveAllEntityRefs();

        //=== Shared Entities part ===

        bool markEntityShared(uint32_t sceneId, Entity entity, fs::path filepath, YAML::Node entityNode);
        bool removeSharedGroup(const std::filesystem::path& filepath);

        std::vector<Entity> importSharedEntity(SceneProject* sceneProject, std::vector<Entity>* entities, const std::filesystem::path& filepath, Entity parent = NULL_ENTITY, bool needSaveScene = true, YAML::Node extendNode = YAML::Node());
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

        bool sharedGroupPropertyChanged(uint32_t sceneId, Entity entity, ComponentType componentType, std::vector<std::string> properties, bool changeItself = false);
        bool sharedGroupNameChanged(uint32_t sceneId, Entity entity, std::string name, bool changeItself = false);

        //=== end Shared Entities part ===

        bool isAnyScenePlaying() const;

        void start(uint32_t sceneId);
        void pause(uint32_t sceneId);
        void resume(uint32_t sceneId);
        void stop(uint32_t sceneId);

        void restoreRuntimeLayers(uint32_t sceneId);

        void debugSceneHierarchy();
    };

}
