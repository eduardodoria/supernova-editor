// SharedGroup.cpp
#include "SharedGroup.h"

namespace Supernova::Editor {

// Helper methods

Entity SharedGroup::getRootEntity(uint32_t sceneId, uint32_t instanceId) const {
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

std::vector<Entity> SharedGroup::getAllEntities(uint32_t sceneId, uint32_t instanceId) const {
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

std::vector<uint32_t> SharedGroup::getInstanceIds(uint32_t sceneId) const {
    std::vector<uint32_t> result;
    auto it = instances.find(sceneId);
    if (it != instances.end()) {
        for (const auto& instance : it->second) {
            result.push_back(instance.instanceId);
        }
    }
    return result;
}

uint32_t SharedGroup::getInstanceId(uint32_t sceneId, Entity entity) const {
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

// Registry/local mapping helpers

Entity SharedGroup::getRegistryEntity(uint32_t sceneId, Entity entity) const {
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

Entity SharedGroup::getLocalEntity(uint32_t sceneId, uint32_t instanceId, Entity registryEntity) const {
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

// Membership and instance access

bool SharedGroup::containsEntity(uint32_t sceneId, Entity entity) const {
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

SharedGroup::Instance* SharedGroup::getInstance(uint32_t sceneId, Entity entity) {
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

const SharedGroup::Instance* SharedGroup::getInstance(uint32_t sceneId, Entity entity) const {
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

SharedGroup::Instance* SharedGroup::getInstanceById(uint32_t sceneId, uint32_t instanceId) {
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

const SharedGroup::Instance* SharedGroup::getInstanceById(uint32_t sceneId, uint32_t instanceId) const {
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

void SharedGroup::removeInstance(uint32_t sceneId, uint32_t instanceId) {
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

// Override management

void SharedGroup::setComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) {
    Instance* instance = getInstanceById(sceneId, instanceId);
    if (instance) {
        uint64_t bit = 1ULL << static_cast<int>(componentType);
        instance->overrides[entity] |= bit;
    }
}

void SharedGroup::setComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        setComponentOverride(sceneId, instanceId, entity, componentType);
    }
}

void SharedGroup::clearComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) {
    Instance* instance = getInstanceById(sceneId, instanceId);
    if (instance) {
        auto entityIt = instance->overrides.find(entity);
        if (entityIt != instance->overrides.end()) {
            uint64_t bit = 1ULL << static_cast<int>(componentType);
            entityIt->second &= ~bit;
            if (entityIt->second == 0) {
                instance->overrides.erase(entityIt);
            }
        }
    }
}

void SharedGroup::clearComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        clearComponentOverride(sceneId, instanceId, entity, componentType);
    }
}

bool SharedGroup::hasComponentOverride(uint32_t sceneId, uint32_t instanceId, Entity entity, ComponentType componentType) const {
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

bool SharedGroup::hasComponentOverride(uint32_t sceneId, Entity entity, ComponentType componentType) const {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        return hasComponentOverride(sceneId, instanceId, entity, componentType);
    }
    return false;
}

uint64_t SharedGroup::getEntityOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
    const Instance* instance = getInstanceById(sceneId, instanceId);
    if (instance) {
        auto entityIt = instance->overrides.find(entity);
        if (entityIt != instance->overrides.end()) {
            return entityIt->second;
        }
    }
    return 0; // No overrides
}

uint64_t SharedGroup::getEntityOverrides(uint32_t sceneId, Entity entity) const {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        return getEntityOverrides(sceneId, instanceId, entity);
    }
    return 0;
}

void SharedGroup::clearAllOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) {
    Instance* instance = getInstanceById(sceneId, instanceId);
    if (instance) {
        instance->overrides.erase(entity);
    }
}

void SharedGroup::clearAllOverrides(uint32_t sceneId, Entity entity) {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        clearAllOverrides(sceneId, instanceId, entity);
    }
}

void SharedGroup::clearAllInstanceOverrides(uint32_t sceneId, uint32_t instanceId) {
    Instance* instance = getInstanceById(sceneId, instanceId);
    if (instance) {
        instance->overrides.clear();
    }
}

void SharedGroup::clearAllSceneOverrides(uint32_t sceneId) {
    auto it = instances.find(sceneId);
    if (it != instances.end()) {
        for (auto& instance : it->second) {
            instance.overrides.clear();
        }
    }
}

bool SharedGroup::hasAnyOverrides(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
    return getEntityOverrides(sceneId, instanceId, entity) != 0;
}

bool SharedGroup::hasAnyOverrides(uint32_t sceneId, Entity entity) const {
    return getEntityOverrides(sceneId, entity) != 0;
}

std::vector<ComponentType> SharedGroup::getOverriddenComponents(uint32_t sceneId, uint32_t instanceId, Entity entity) const {
    std::vector<ComponentType> result;
    uint64_t overrideMask = getEntityOverrides(sceneId, instanceId, entity);
    for (int i = 0; i < 64; ++i) {
        if (overrideMask & (1ULL << i)) {
            result.push_back(static_cast<ComponentType>(i));
        }
    }
    return result;
}

std::vector<ComponentType> SharedGroup::getOverriddenComponents(uint32_t sceneId, Entity entity) const {
    uint32_t instanceId = getInstanceId(sceneId, entity);
    if (instanceId != 0) {
        return getOverriddenComponents(sceneId, instanceId, entity);
    }
    return {};
}

// Aggregates

bool SharedGroup::hasInstances(uint32_t sceneId) const {
    auto it = instances.find(sceneId);
    return it != instances.end() && !it->second.empty();
}

size_t SharedGroup::getTotalInstanceCount() const {
    size_t count = 0;
    for (const auto& kv : instances) {
        count += kv.second.size();
    }
    return count;
}

std::vector<Entity> SharedGroup::getAllSceneEntities(uint32_t sceneId) const {
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

} // namespace Supernova::Editor
