#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Factory.h"

#define NULL_PROJECT_SCENE 0

namespace Supernova::Editor{

    struct SceneData{
        uint32_t id;
        std::string name;
        Scene* scene;
        std::vector<Entity> entities;
    };

    class Project{
    private:

        static uint32_t nextSceneId;

        std::vector<SceneData> scenes;

    public:
        Project();

        void createNewScene(std::string sceneName);
        Entity createNewEntity(std::string sceneName);
        bool createNewComponent(std::string sceneName, Entity entity, ComponentType component);

        std::vector<SceneData>& getScenes();
    };

}

#endif /* PROJECT_H */