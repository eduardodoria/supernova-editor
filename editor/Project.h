#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Catalog.h"
#include "render/SceneRender.h"
#include "command/CommandHistory.h"
#include "Conector.h"
#include "Generator.h"

#include <filesystem>

#define NULL_PROJECT_SCENE 0

namespace Supernova::Editor{

    struct SceneProject{
        uint32_t id;
        std::string name;
        Scene* scene;
        std::vector<Entity> entities;
        SceneRender* sceneRender;
        std::vector<Entity> selectedEntities;
        bool needUpdateRender;
    };

    class Project{
    private:

        Conector conector;
        Generator generator;

        static uint32_t nextSceneId;

        std::vector<SceneProject> scenes;
        uint32_t selectedScene;

        uint32_t lastActivatedScene;

        bool tempPath;
        std::filesystem::path projectPath;
        bool resourcesFocused;

        template<typename T>
        T* findScene(uint32_t sceneId) const;

        Entity createNewEntity(uint32_t sceneId, std::string entityName);
        bool createNewComponent(uint32_t sceneId, Entity entity, ComponentType component);

    public:
        Project();

        void saveProject();

        bool createNewProject(std::string projectName);
        uint32_t createNewScene(std::string sceneName);

        void deleteEntity(uint32_t sceneId, Entity entity);
        void deleteEntities(uint32_t sceneId, std::vector<Entity> entities);

        Entity findObjectByRay(uint32_t sceneId, float x, float y);

        bool selectObjectByRay(uint32_t sceneId, float x, float y, bool shiftPressed);
        bool selectObjectsByRect(uint32_t sceneId, Vector2 start, Vector2 end);

        std::vector<SceneProject>& getScenes();
        SceneProject* getScene(uint32_t sceneId);
        const SceneProject* getScene(uint32_t sceneId) const;
        SceneProject* getSelectedScene();
        const SceneProject* getSelectedScene() const;

        void setSelectedSceneId(uint32_t selectedScene);
        uint32_t getSelectedSceneId() const;

        bool isTempPath() const;
        std::filesystem::path getProjectPath() const;

        void setLastActivatedSceneId(uint32_t lastActivatedScene);
        uint32_t getLastActivatedSceneId() const;

        void replaceSelectedEntities(uint32_t sceneId, std::vector<Entity> selectedEntities);
        void setSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        void addSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        bool isSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        void clearSelectedEntities(uint32_t sceneId);
        std::vector<Entity> getSelectedEntities(uint32_t sceneId) const;
        bool hasSelectedEntities(uint32_t sceneId) const;

        void build();
    };

}

#endif /* PROJECT_H */