#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Factory.h"
#include "render/SceneRender.h"

#define NULL_PROJECT_SCENE 0

namespace Supernova::Editor{

    struct SceneData{
        uint32_t id;
        std::string name;
        Scene* scene;
        std::vector<Entity> entities;
        SceneRender* sceneRender;
        Entity selectedEntity;
        bool needUpdateRender;
    };

    class Project{
    private:

        static uint32_t nextSceneId;

        std::vector<SceneData> scenes;

        uint32_t selectedScene;

        template<typename T>
        T* findScene(uint32_t sceneId) const;

    public:
        Project();

        uint32_t createNewScene(std::string sceneName);
        Entity createNewEntity(uint32_t sceneId, std::string entityName);
        bool createNewComponent(uint32_t sceneId, Entity entity, ComponentType component);

        void createBoxShape(uint32_t sceneId);

        bool findObjectByRay(uint32_t sceneId, float x, float y);

        std::vector<SceneData>& getScenes();
        SceneData* getScene(uint32_t sceneId);
        const SceneData* getScene(uint32_t sceneId) const;
        SceneData* getSelectedScene();
        const SceneData* getSelectedScene() const;

        void setSelectedSceneId(uint32_t selectedScene);
        uint32_t getSelectedSceneId() const;

        void setSelectedEntity(uint32_t sceneId, Entity selectedEntity);
        Entity getSelectedEntity(uint32_t sceneId) const;
    };

}

#endif /* PROJECT_H */