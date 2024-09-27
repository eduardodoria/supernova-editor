#ifndef PROJECT_H
#define PROJECT_H

#include "Scene.h"
#include "Factory.h"

namespace Supernova::Editor{

    struct SceneData{
        std::string name;
        Scene* scene;
        std::vector<Entity> entities;
    };

    class Project{
    private:

        std::vector<SceneData> scenes;

    public:
        Project();

        void createNewScene(std::string sceneName);
        Entity createNewEntity(std::string sceneName);
        void createNewComponent(std::string sceneName, Entity entity, ComponentType component);
    };

}

#endif /* PROJECT_H */