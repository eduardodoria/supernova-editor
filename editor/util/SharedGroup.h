// SharedGroup.h
#pragma once

#include <map>
#include <vector>
#include <memory>
#include <cstdint>
#include <algorithm>

#include "Scene.h"     // Entity, EntityRegistry, NULL_ENTITY
#include "Catalog.h"   // ComponentType

namespace Supernova::Editor {

class SharedGroup {
public:
    struct EntityMember {
        Entity localEntity;     // Entity in the scene
        Entity registryEntity;  // Corresponding entity in the shared registry
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
    Entity getRootEntity(uint32_t sceneId, uint32_t instanceId) const;
    std::vector<Entity> getAllEntities(uint32_t sceneId, uint32_t instanceId) const;
    std::vector<uint32_t> getInstanceIds(uint32_t sceneId) const;
    uint32_t getInstanceId(uint32_t sceneId, Entity entity) const;

    // Registry/local mapping helpers
    Entity getRegistryEntity(uint32_t sceneId, Entity entity) const;
    Entity getLocalEntity(uint32_t sceneId, uint32_t instanceId, Entity registryEntity) const;

    // Membership and instance access
    bool containsEntity(uint32_t sceneId, Entity entity) const;
    Instance* getInstance(uint32_t sceneId, Entity entity);
    const Instance* getInstance(uint32_t sceneId, Entity entity) const;
    Instance* getInstanceById(uint32_t sceneId, uint32_t instanceId);
    const Instance* getInstanceById(uint32_t sceneId, uint32_t instanceId) const;
    void removeInstance(uint32_t sceneId, uint32_t instanceId);

    // Override management
    void setComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType);
    void setComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType);

    void clearComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType);
    void clearComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType);

    bool hasComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) const;
    bool hasComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) const;

    uint64_t getEntityOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const;
    uint64_t getEntityOverrides(uint32_t sceneId, Entity entity) const;

    void clearAllOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity);
    void clearAllOverrides(uint32_t sceneId, Entity entity);
    void clearAllInstanceOverrides(uint32_t sceneId, uint32_t instanceId);
    void clearAllSceneOverrides(uint32_t sceneId);

    bool hasAnyOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const;
    bool hasAnyOverrides(uint32_t sceneId, Entity entity) const;

    std::vector<ComponentType> getOverriddenComponents(uint32_t sceneId, uint32_t instanceId, Entity entity) const;
    std::vector<ComponentType> getOverriddenComponents(uint32_t sceneId, Entity entity) const;

    // Aggregates
    bool hasInstances(uint32_t sceneId) const;
    size_t getTotalInstanceCount() const;
    std::vector<Entity> getAllSceneEntities(uint32_t sceneId) const;
};

} // namespace Supernova::Editor

