#pragma once

#include "command/Command.h"
#include "Project.h"
#include "yaml-cpp/yaml.h"

namespace Supernova::Editor{

    class ModelLoadCmd: public Command{

    private:
        YAML::Node oldTransform;
        YAML::Node oldMesh;
        YAML::Node oldModel;
        YAML::Node skeleton;
        std::vector<YAML::Node> oldAnimations;

        bool hasSkeleton = false;

        Project* project;
        uint32_t sceneId;
        Entity entity;

        std::string modelPath;
        std::string oldModelPath;

        std::vector<Entity> addedEntities;
        std::vector<Entity> oldAddedEntities;

        bool wasModified;

        void collectModelEntities(Scene* scene, const ModelComponent& model, std::vector<Entity>& out);
        void addEntitiesToScene(SceneProject* sceneProject, const std::vector<Entity>& ents);
        void removeEntitiesFromScene(SceneProject* sceneProject, const std::vector<Entity>& ents);

    public:
        ModelLoadCmd(Project* project, uint32_t sceneId, Entity entity, const std::string& modelPath);

        bool execute() override;
        void undo() override;

        bool mergeWith(Command* otherCommand) override;
    };

}
