#pragma once

#include "yaml-cpp/yaml.h"
#include "Scene.h"
#include "ecs/Entity.h"
#include "Project.h"
#include "math/Vector3.h"
#include "math/Quaternion.h"
#include "math/Matrix4.h"

#include <unordered_map>
#include <unordered_set>

namespace Supernova::Editor {
    class Stream {
    private:

        static std::string sceneTypeToString(SceneType type);
        static SceneType stringToSceneType(const std::string& str);

        static std::string primitiveTypeToString(PrimitiveType type);
        static PrimitiveType stringToPrimitiveType(const std::string& str);

        static std::string bufferTypeToString(BufferType type);
        static BufferType stringToBufferType(const std::string& str);

        static std::string bufferUsageToString(BufferUsage usage);
        static BufferUsage stringToBufferUsage(const std::string& str);

        static std::string attributeTypeToString(AttributeType type);
        static AttributeType stringToAttributeType(const std::string& str);

        static std::string attributeDataTypeToString(AttributeDataType type);
        static AttributeDataType stringToAttributeDataType(const std::string& str);

        static std::string cullingModeToString(CullingMode mode);
        static CullingMode stringToCullingMode(const std::string& str);

        static std::string windingOrderToString(WindingOrder order);
        static WindingOrder stringToWindingOrder(const std::string& str);

        static std::string textureFilterToString(TextureFilter filter);
        static TextureFilter stringToTextureFilter(const std::string& str);

        static std::string textureWrapToString(TextureWrap wrap);
        static TextureWrap stringToTextureWrap(const std::string& str);

        static std::string lightTypeToString(LightType type);
        static LightType stringToLightType(const std::string& str);

        static std::string lightStateToString(LightState state);
        static LightState stringToLightState(const std::string& str);

        static std::string pivotPresetToString(PivotPreset preset);
        static PivotPreset stringToPivotPreset(const std::string& str);

        // ==============================

        static YAML::Node encodeVector2(const Vector2& vec);
        static Vector2 decodeVector2(const YAML::Node& node);

        static YAML::Node encodeVector3(const Vector3& vec);
        static Vector3 decodeVector3(const YAML::Node& node);

        static YAML::Node encodeVector4(const Vector4& vec);
        static Vector4 decodeVector4(const YAML::Node& node);

        static YAML::Node encodeQuaternion(const Quaternion& quat);
        static Quaternion decodeQuaternion(const YAML::Node& node);

        static YAML::Node encodeRect(const Rect& rect);
        static Rect decodeRect(const YAML::Node& node);

        static YAML::Node encodeMatrix4(const Matrix4& mat);
        static Matrix4 decodeMatrix4(const YAML::Node& node);

        static YAML::Node encodeTexture(const Texture& texture);
        static Texture decodeTexture(const YAML::Node& node);

        static YAML::Node encodeBuffer(const Buffer& buffer);
        static void decodeBuffer(Buffer& buffer, const YAML::Node& node);

        static YAML::Node encodeInterleavedBuffer(const InterleavedBuffer& buffer);
        static void decodeInterleavedBuffer(InterleavedBuffer& buffer, const YAML::Node& node);

        static YAML::Node encodeIndexBuffer(const IndexBuffer& buffer);
        static void decodeIndexBuffer(IndexBuffer& buffer, const YAML::Node& node);

        static YAML::Node encodeExternalBuffer(const ExternalBuffer& buffer);
        static void decodeExternalBuffer(ExternalBuffer& buffer, const YAML::Node& node);

        static YAML::Node encodeSubmesh(const Submesh& submesh);
        static Submesh decodeSubmesh(const YAML::Node& node);

        static YAML::Node encodeAABB(const AABB& aabb);
        static AABB decodeAABB(const YAML::Node& node);

        static YAML::Node encodeSpriteFrameData(const SpriteFrameData& frameData);
        static SpriteFrameData decodeSpriteFrameData(const YAML::Node& node);

        static YAML::Node encodeEntityAux(const Entity entity, const EntityRegistry* registry, const Project* project = nullptr, const SceneProject* sceneProject = nullptr, bool keepEntity = false);

        static YAML::Node encodeScriptProperty(const ScriptProperty& prop, const Project* project, const SceneProject* sceneProject);
        static ScriptProperty decodeScriptProperty(const YAML::Node& node, Project* project, SceneProject* sceneProject, Entity ownerEntity, size_t scriptIndex, size_t propertyIndex);

    public:
        static YAML::Node encodeProject(Project* project);
        static void decodeProject(Project* project, const YAML::Node& node);

        static YAML::Node encodeSceneProject(const Project* project, const SceneProject* sceneProject, bool keepEntity = false);
        static void decodeSceneProject(SceneProject* sceneProject, const YAML::Node& node);
        static void decodeSceneProjectEntities(Project* project, SceneProject* sceneProject, const YAML::Node& node);

        static YAML::Node encodeScene(Scene* scene);
        static Scene* decodeScene(Scene* scene, const YAML::Node& node);

        static YAML::Node encodeEntity(const Entity entity, const EntityRegistry* registry, const Project* project = nullptr, const SceneProject* sceneProject = nullptr, bool keepEntity = false);
        static std::vector<Entity> decodeEntity(const YAML::Node& entityNode, EntityRegistry* registry, std::vector<Entity>* entities = nullptr, Project* project = nullptr, SceneProject* sceneProject = nullptr, Entity parent = NULL_ENTITY, bool returnSharedEntities = true, bool createNewIfExists = true);

        static YAML::Node encodeMaterial(const Material& material);
        static Material decodeMaterial(const YAML::Node& node);

        static YAML::Node encodeEntityRef(const EntityRef& ref, const Project* project = nullptr, const SceneProject* currentSceneProject = nullptr);
        static EntityRef decodeEntityRef(const YAML::Node& node, Project* project = nullptr, SceneProject* currentSceneProject = nullptr);

        static YAML::Node encodeComponents(const Entity entity, const EntityRegistry* registry, const Project* project, const SceneProject* sceneProject, Signature signature);
        static void decodeComponents(Entity entity, Entity parent, EntityRegistry* registry, Project* project, SceneProject* sceneProject, const YAML::Node& compNode);

        static YAML::Node encodeTransform(const Transform& transform);
        static Transform decodeTransform(const YAML::Node& node, const Transform* oldTransform = nullptr);

        static YAML::Node encodeMeshComponent(const MeshComponent& mesh);
        static MeshComponent decodeMeshComponent(const YAML::Node& node, const MeshComponent* oldMesh = nullptr);

        static YAML::Node encodeUIComponent(const UIComponent& ui);
        static UIComponent decodeUIComponent(const YAML::Node& node, const UIComponent* oldUI = nullptr);

        static YAML::Node encodeUILayoutComponent(const UILayoutComponent& layout);
        static UILayoutComponent decodeUILayoutComponent(const YAML::Node& node, const UILayoutComponent* oldLayout = nullptr);

        static YAML::Node encodeImageComponent(const ImageComponent& image);
        static ImageComponent decodeImageComponent(const YAML::Node& node, const ImageComponent* oldImage = nullptr);

        static YAML::Node encodeSpriteComponent(const SpriteComponent& sprite);
        static SpriteComponent decodeSpriteComponent(const YAML::Node& node, const SpriteComponent* oldSprite = nullptr);

        static YAML::Node encodeLightComponent(const LightComponent& light);
        static LightComponent decodeLightComponent(const YAML::Node& node, const LightComponent* oldLight = nullptr);

        static YAML::Node encodeScriptComponent(const ScriptComponent& script, const Project* project = nullptr, const SceneProject* sceneProject = nullptr);
        static ScriptComponent decodeScriptComponent(const YAML::Node& node, const ScriptComponent* oldScript = nullptr, Project* project = nullptr, SceneProject* sceneProject = nullptr, Entity ownerEntity = NULL_ENTITY);
    };
}