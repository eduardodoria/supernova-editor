#pragma once

#include "lua.hpp"

#include "Project.h"
#include "script/ScriptProperty.h"

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

    // --- Lua script utilities ---

    static ScriptPropertyType stringToScriptPropertyType(const std::string& s);
    static ScriptPropertyValue luaValueToScriptPropertyValue(lua_State* L, int idx, ScriptPropertyType type);
    static void loadLuaScriptProperties(ScriptEntry& entry, const std::string& luaPath);
};

} // namespace Supernova::Editor
