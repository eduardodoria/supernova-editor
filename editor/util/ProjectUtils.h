#pragma once

#include "Project.h"

namespace Supernova::Editor {

class ProjectUtils {
public:
    static size_t getTransformIndex(EntityRegistry* registry, Entity entity);
    static void sortEntitiesByTransformOrder(EntityRegistry* registry, std::vector<Entity>& entities);

    static bool moveEntityOrderByTarget(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, bool& hasTransform);
    static void moveEntityOrderByIndex(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t index, bool hasTransform);
    static void moveEntityOrderByTransform(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t transformIndex, bool enableMove = true);

    static void addEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, YAML::Node componentNode = YAML::Node());
    static YAML::Node removeEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, bool encodeComponent = false);
};

} // namespace Supernova::Editor
