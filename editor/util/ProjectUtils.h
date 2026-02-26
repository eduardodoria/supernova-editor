#pragma once

#include "lua.hpp"

#include <string>

#include "LuaBridge.h"
#include "Out.h"

#include "Project.h"
#include "script/ScriptProperty.h"

#include "texture/Texture.h"

namespace Supernova::Editor {

class ProjectUtils {
private:
    template <typename T>
    static bool pushEntityHandleTyped(lua_State* L, Scene* scene, Entity entity, const char* typeNameForLog) {
        T* handle = new T(scene, entity);
        printf("[DEBUG]   No Lua script instance found, creating '%s' type\n", typeNameForLog);

        if (!luabridge::push<T*>(L, handle)) {
            delete handle;
            Out::error("Failed to push %s for EntityRef property", typeNameForLog);
            lua_pushnil(L);
            return false;
        }

        return true;
    }

public:

    static bool isEntityLocked(Scene* scene, Entity entity);

    static size_t getTransformIndex(EntityRegistry* registry, Entity entity);
    static void sortEntitiesByTransformOrder(EntityRegistry* registry, std::vector<Entity>& entities);

    static bool moveEntityOrderByTarget(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity target, InsertionType type, Entity& oldParent, size_t& oldIndex, bool& hasTransform);
    static void moveEntityOrderByIndex(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t index, bool hasTransform);
    static void moveEntityOrderByTransform(EntityRegistry* registry, std::vector<Entity>& entities, Entity source, Entity parent, size_t transformIndex, bool enableMove = true);

    static void addEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, YAML::Node componentNode = YAML::Node());
    static YAML::Node removeEntityComponent(EntityRegistry* registry, Entity entity, ComponentType componentType, std::vector<Entity>& entities, bool encodeComponent = false);

    // --- Lua script utilities ---
    static ScriptPropertyValue luaValueToScriptPropertyValue(lua_State* L, int idx, ScriptPropertyType type);
    static void loadLuaScriptProperties(ScriptEntry& entry, const std::string& luaPath);

    // Fills a Texture with the editor built-in default skybox cubemap.
    static void setDefaultSkyTexture(Texture& outTexture);
};

} // namespace Supernova::Editor
